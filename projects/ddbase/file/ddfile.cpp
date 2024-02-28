#include "ddbase/stdafx.h"
#include "ddbase/file/ddfile.h"
#include "ddbase/file/dddir.h"
#include "ddbase/ddlog.hpp"

#include <windows.h>

namespace NSP_DD {
ddfile::ddfile()
{
}

ddfile::~ddfile()
{
    if (m_file_handle != NULL) {
        ::CloseHandle(m_file_handle);
        m_file_handle = NULL;
    }
}

ddfile* ddfile::create_or_open(const std::wstring& path, bool read_only /* = false */)
{
    HANDLE file_handle = ::CreateFileW(path.c_str(),
        GENERIC_READ | (read_only ? 0 : GENERIC_WRITE),
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        DDLOG1W(ERROR, L"CreateFileW failure\n");
        DDLOG1W(ERROR, path.c_str());
        DDLOG_LASTERROR();
        
        return nullptr;
    }
    ddfile* file = new ddfile();
    file->m_file_handle = file_handle;
    file->m_fullpath = path;
    if (!file->seek(0)) {
        delete file;
        return nullptr;
    }
    return file;
}

ddfile* ddfile::create_file_with_head_checker(const std::wstring& path, const u8* head_checker, s32 head_checker_size, bool read_only /* = false */)
{
    std::unique_ptr<ddfile> file(ddfile::create_or_open(path, read_only));
    if (file == nullptr) {
        return nullptr;
    }

    if (head_checker == nullptr || head_checker_size == 0) {
        return file.release();
    }

    s64 file_size = file->file_size();
    if (file_size == 0) {
        // 文件为空, 写入文件头
        if ((s32)file->write(head_checker, head_checker_size) != head_checker_size) {
            DDLOG1W(ERROR, L"write file header failure.\n");
            return nullptr;
        }
        return file.release();
    }
    
    if (file_size < head_checker_size) {
        DDLOG1W(ERROR, L"the file has exit and is not match head_checker.\n");
        return nullptr;
    }

    // 检查文件头
    ddbuff buff(head_checker_size);
    if (file->read(buff.data(), head_checker_size) != head_checker_size) {
        return nullptr;
    }

    for (int i = 0; i < head_checker_size; ++i) {
        if (head_checker[i] != buff[i]) {
            DDLOG1W(ERROR, L"the file has exit and is not match head_checker.\n");
            return nullptr;
        }
    }
    if (!file->seek(head_checker_size)) {
        return nullptr;
    }
    return file.release();
}

static u8 utf8_bom_header[] = { 0xef, 0xbb, 0xbf };
static u8 usc2_le_header[] = { 0xff, 0xfe };
static u8 usc2_be_header[] = { 0xfe, 0xff };

ddfile* ddfile::create_usc2_file(const std::wstring& path, bool read_only /* = false */)
{
    return create_file_with_head_checker(path, usc2_le_header, sizeof(usc2_le_header), read_only);
}

ddfile* ddfile::create_usc2_file_be(const std::wstring& path, bool read_only /* = false */)
{
    return create_file_with_head_checker(path, usc2_be_header, sizeof(usc2_be_header), read_only);
}

ddfile* ddfile::create_utf8_file(const std::wstring& path, bool read_only /* = false */)
{
    return create_file_with_head_checker(path, nullptr, 0, read_only);
}

ddfile* ddfile::create_utf8bom_file(const std::wstring& path, bool read_only /* = false */)
{
    return create_file_with_head_checker(path, utf8_bom_header, sizeof(utf8_bom_header), read_only);
}

ddfile_type ddfile::get_file_type(const std::wstring& path)
{
    std::unique_ptr<ddfile> file(ddfile::create_or_open(path));
    if (file == nullptr) {
        return ddfile_type::unknown;
    }

    u8 header[3] = { 0, 0, 0 };
    (void)file->read(header, 3);
    if (header[0] == utf8_bom_header[0] &&
        header[1] == utf8_bom_header[1] &&
        header[2] == utf8_bom_header[2]) {
        return ddfile_type::utf8bom;
    }

    if (header[0] == usc2_le_header[0] &&
        header[1] == usc2_le_header[1]) {
        return ddfile_type::utf16le;
    }

    if (header[0] == usc2_be_header[0] &&
        header[1] == usc2_be_header[1]) {
        return ddfile_type::utf16be;
    }

    return ddfile_type::utf8_or_ansi;
}

bool ddfile::try_check_is_txt_file(ddfile* src)
{
    if (src == nullptr) {
        return false;
    }
    u8 buff[1024] = { 0 };
    s32 readed = src->read(buff, sizeof(buff));
    for (s32 i = 0; i < readed - 1; ++i) {
        u8 c = buff[i];
        u8 c1 = buff[i + 1];
        if (c == 0 && (c1 > 0x80 || i % 2 == 0)) {
            return false;
        }
    }
    return true;
}

s64 ddfile::file_find(ddfile* src, const ddbuff& finder)
{
    if (src == nullptr || finder.size() > 1024 * 1024) {
        return -1;
    }

    // 10M
    const s32 buff_size = 1024 * 1024 * 10;
    ddbuff buff(buff_size);
    s32 has_checked_size = 0;
    s32 end_size = 0;
    while (true) {
        s32 readed = src->read(buff.data() + end_size, buff_size - end_size) + end_size;
        if (readed < (s32)finder.size()) {
            break;
        }

        buff.resize(readed);
        s32 pos = ddstr::buff_find(buff, finder);
        if (pos != -1) {
            return pos + has_checked_size;
        }

        end_size = s32(finder.size() - 1);
        (void)::memcpy_s(buff.data(), buff_size, &buff[readed - end_size], end_size);
        has_checked_size += (readed - end_size);
        buff.resize(buff_size);
    }
    return -1;
}

bool ddfile::file_replace(ddfile* src, ddfile* dst, const std::vector<ddbuff>& finder, const std::vector<ddbuff>& replacer)
{
    if (src == nullptr || dst == nullptr || finder.size() != replacer.size() || finder.empty()) {
        return false;
    }

    size_t max_finder_lenth = 0;
    for (const auto& it : finder) {
        if (max_finder_lenth < it.size()) {
            max_finder_lenth = it.size();
        }
    }

    // 5M
    const s32 buff_size = 1024 * 1024 * 5;
    ddbuff read_buff(buff_size);
    ddbuff out_buff(buff_size);
    s32 end_size = 0;
    while (true) {
        s32 readed = src->read(read_buff.data() + end_size, buff_size - end_size) + end_size;
        if (readed == end_size) {
            if (readed != 0) {
                if (dst->write(read_buff.data(), readed) != readed) {
                    return false;
                }
            }
            break;
        }

        read_buff.resize(readed);
        end_size = readed - ddstr::buff_replace_ex(read_buff, finder, replacer, out_buff);
        if (end_size > (s32)max_finder_lenth - 1) {
            end_size = (s32)max_finder_lenth - 1;
        }
        (void)::memcpy_s(read_buff.data(), buff_size, &read_buff[readed - end_size], end_size);
        if (end_size < (s32)out_buff.size() && dst->write(out_buff.data(), (s32)out_buff.size() - end_size) != (s32)out_buff.size() - end_size) {
            return false;
        }
        read_buff.resize(buff_size);
    }
    return true;
}

s32 ddfile::write(const u8* buff, s32 size)
{
    DDASSERT(m_file_handle != NULL);
    DDASSERT(buff != NULL);
    DDASSERT(size > 0);
    DWORD writed = 0;
    if (!::WriteFile(m_file_handle, (LPCVOID)buff, (DWORD)size, &writed, NULL)) {
        DDLOG1W(ERROR, L"WriteFile failure\n");
        DDLOG_LASTERROR();
        return 0;
    }
    return s32(writed);
}

bool ddfile::flush()
{
    DDASSERT(m_file_handle != NULL);
    if (::FlushFileBuffers(m_file_handle)) {
        return true;
    }
    DDLOG1W(ERROR, L"FlushFileBuffers failure\n");
    DDLOG_LASTERROR();
    return false;
}

s32 ddfile::read(u8* buff, s32 size)
{
    DDASSERT(m_file_handle != NULL);
    DDASSERT(buff != NULL);
    DDASSERT(size > 0);
    DWORD readed = 0;
    if (!::ReadFile(m_file_handle, (LPVOID)buff, (DWORD)size, &readed, NULL)) {
        DDLOG1W(ERROR, L"ReadFile failure\n");
        DDLOG_LASTERROR();
        return 0;
    }
    return readed;
}

bool ddfile::read_linew(std::wstring& line)
{
    line.clear();
    read_untilw([&line](wchar_t c) {
        line.append(1, (wchar_t)c);
        return c == L'\n';
    });

    return line.length() > 0;
}

bool ddfile::read_linea(std::string& line)
{
    line.clear();
    read_untila([&line](char c) {
        line.append(1, (u8)c);
        return c == '\n';
    });

    return line.length() > 0;
}

void ddfile::read_untila(std::function<bool(char ch)> until)
{
    DDASSERT(m_file_handle != NULL);
    DDASSERT(until != NULL);
    u8 buff[256];
    while (true) {
        s32 readed = read(buff, sizeof(buff));
        if (readed == 0) {
            return;
        }

        for (s32 i = 0; i < readed; ++i) {
            if (until((char)buff[i])) {
                (void)seek(current_offset() + i - readed + 1);
                return;
            }
        }
    }
}

void ddfile::read_untilw(std::function<bool(wchar_t ch)> until)
{
    DDASSERT(m_file_handle != NULL);
    DDASSERT(until != NULL);
    u16 buff[256] = {0};
    while (true) {
        s32 readed = read((u8*)buff, sizeof(buff));
        if (readed % 2 == 1) {
            (void)seek(current_offset() - 1);
        }
        readed = readed / 2;
        if (readed == 0) {
            return;
        }

        for (s32 i = 0; i < readed; ++i) {
            if (until((wchar_t)buff[i])) {
                (void)seek(current_offset() + 2 * i - 2 * readed + 2);
                return;
            }
        }
    }
}

bool ddfile::resize(s64 size /* =0 */)
{
    DDASSERT(m_file_handle != NULL);
    DDASSERT(size >= 0);
    if (!seek(size)) {
        return false;
    }

    if (::SetEndOfFile(m_file_handle)) {
        return true;
    }

    DDLOG1W(ERROR, L"ReadFile failure\n");
    DDLOG_LASTERROR();
    return false;
}

bool ddfile::seek(s64 offset)
{
    DDASSERT(m_file_handle != NULL);
    LARGE_INTEGER distance_to_move { 0 };
    if (offset == -1) {
        if (!::SetFilePointerEx(m_file_handle, distance_to_move, NULL, FILE_END)) {
            DDLOG1W(ERROR, L"SetFilePointerEx failure\n");
            DDLOG_LASTERROR();
            return false;
        }
        return true;
    }

    DDASSERT(offset >= 0);
    distance_to_move.QuadPart = (LONGLONG)offset;
    if (!::SetFilePointerEx(m_file_handle, distance_to_move, NULL, FILE_BEGIN)) {
        DDLOG1W(ERROR, L"SetFilePointerEx failure\n");
        DDLOG_LASTERROR();
        return false;
    }
    return true;
}

s64 ddfile::current_offset()
{
    LARGE_INTEGER distance_to_move { 0 };
    LARGE_INTEGER new_distance { 0 };
    if (!::SetFilePointerEx(m_file_handle, distance_to_move, &new_distance, FILE_CURRENT)) {
        return s64(0);
    }
    return (s64)new_distance.QuadPart;
}

s64 ddfile::file_size()
{
    DDASSERT(m_file_handle != NULL);
    ::LARGE_INTEGER large_integer;
    if (!::GetFileSizeEx(m_file_handle, &large_integer)) {
        DDLOG1W(ERROR, L"GetFileSizeEx failure\n");
        DDLOG_LASTERROR();
        return 0;
    }

    return (s64)large_integer.QuadPart;
}

s64 ddfile::file_size(const std::wstring& full_path)
{
    ::WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
    if (::GetFileAttributesExW(
        full_path.c_str(),
        ::_GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard,
        &fileAttributeData)) {
        ::LARGE_INTEGER fileSize;
        fileSize.HighPart = fileAttributeData.nFileSizeHigh;
        fileSize.LowPart = fileAttributeData.nFileSizeLow;
        return (s64)fileSize.QuadPart;
    }
    return 0;
}

const std::wstring& ddfile::get_fullpath()
{
    return m_fullpath;
}
} // namespace NSP_DD
