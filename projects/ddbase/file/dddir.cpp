
#include "ddbase/stdafx.h"
#include "ddbase/file/dddir.h"
#include "ddbase/file/ddpath.h"

#include <queue>
#include <memory>

#include <Windows.h>
namespace NSP_DD {

bool dddir::is_dir(const std::wstring& path)
{
    DWORD ftyp = ::GetFileAttributes(path.c_str());

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
        // 这是一个文件夹
        return true;
    }

    return false;
}

bool dddir::is_path_exist(const std::wstring& file_path)
{
    DWORD dwAttrib = ::GetFileAttributes(file_path.c_str());
    return (INVALID_FILE_ATTRIBUTES != dwAttrib);
}

bool dddir::create_file(const std::wstring& file_path)
{
    DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    DWORD dwShareMode = 0;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes = 0;
    DWORD dwCreationDisposition = CREATE_NEW;
    DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    HANDLE hTemplateFile = 0;
    HANDLE handle = CreateFile(file_path.c_str(),
                               dwDesiredAccess,
                               dwShareMode,
                               lpSecurityAttributes,
                               dwCreationDisposition,
                               dwFlagsAndAttributes,
                               hTemplateFile);

    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    ::CloseHandle(handle);
    return true;
}

static bool delete_file(const std::wstring& path)
{
    return !!::DeleteFile(path.c_str());
}

static bool delete_empty_dir(const std::wstring& path)
{
    return !!::RemoveDirectoryW(path.c_str());
}

bool dddir::delete_path(const std::wstring& path)
{
    if (!is_path_exist(path)) {
        return false;
    }

    if (!is_dir(path)) {
        return delete_file(path);
    }

    WIN32_FIND_DATA fileInfo{ 0 };
    HANDLE file = ::FindFirstFile(ddpath::join(path, L"*").c_str(), &fileInfo);
    if (INVALID_HANDLE_VALUE == file) {
        return false;
    }

    do {
        std::wstring name = fileInfo.cFileName;
        if (name == L"." || name == L"..") {
            continue;
        }

        if (!delete_path(ddpath::join(path, name))) {
            return false;
        }
    } while (::FindNextFile(file, &fileInfo));

    (void)::FindClose(file);
    return delete_empty_dir(path);
}

bool dddir::create_dir(const std::wstring& dir_path)
{
    if (is_path_exist(dir_path)) {
        return true;
    }

    return ::CreateDirectory(dir_path.c_str(), nullptr) == TRUE;
}

bool dddir::create_dir_ex(const std::wstring& dir_path)
{
    if (dir_path.empty()) {
        return false;
    }

    if (is_path_exist(dir_path)) {
        return true;
    }

    std::wstring tmp = L"";
    tmp.resize(dir_path.size(), 0);

    for (size_t i = 0; i < dir_path.length(); ++i) {
        if (dir_path[i] == L'\\' || dir_path[i] == L'/') {
            if (!create_dir(tmp)) {
                return false;
            }
        }

        tmp[i] = dir_path[i];
    }

    if (tmp[tmp.size() - 1] != L'\\' && tmp[tmp.size() - 1] != L'/') {
        if (!create_dir(tmp)) {
            return false;
        }
    }
    return true;
}

bool dddir::rename_path(const std::wstring& src_path, const std::wstring& dst_path)
{
    if (!is_path_exist(src_path)) {
        return false;
    }

    if (::MoveFileExW(src_path.c_str(), dst_path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {
        return false;
    }
    return true;
}

static bool copy_file(const std::wstring& src_path, const std::wstring& dst_path)
{
    if (!dddir::create_dir_ex(ddpath::parent(dst_path))) {
        return false;
    }
    return (::CopyFile(src_path.c_str(), dst_path.c_str(), FALSE) != 0);
}

bool dddir::copy_path(const std::wstring& src_path, const std::wstring& dst_path)
{
    return copy_path_ex(src_path, dst_path);
}

bool dddir::copy_path_ex(const std::wstring& src_path,
    const std::wstring& dst_path,
    const std::function<bool(const std::wstring&)>& exclude_filter /* = nullptr */)
{
    if (!is_path_exist(src_path)) {
        return false;
    }

    if (!is_dir(src_path)) {
        if (exclude_filter != nullptr && exclude_filter(src_path)) {
            return true;
        }
        return copy_file(src_path, dst_path);
    }

    bool success = true;
    enum_dir(src_path, [&success, &dst_path, &src_path, &exclude_filter](const std::wstring& full_path, bool is_dir) {
        if (exclude_filter != nullptr && exclude_filter(full_path)) {
            return false;
        }

        std::wstring new_path = ddpath::join(dst_path, full_path.substr(src_path.length() + 1));
        if (is_dir) {
            if (!dddir::create_dir_ex(new_path)) {
                success = false;
                return true;
            }
            return false;
        }

        if (!copy_path(full_path, new_path)) {
            success = false;
            return true;
        }
        return false;
    });
    return success;
}

void dddir::enum_dir_first_level(const std::wstring& dir_path, std::function<bool(const std::wstring& name, bool is_dir)> call_back)
{
    std::wstring root_path = dir_path;
    ddpath::normal(root_path);
    if (call_back == nullptr || !is_path_exist(root_path) || !is_dir(root_path)) {
        return;
    }

    // 枚举路径
    WIN32_FIND_DATA file_info{ 0 };
    HANDLE hFile = ::FindFirstFile(ddpath::join(root_path, L"*").c_str(), &file_info);
    if (INVALID_HANDLE_VALUE == hFile) {
        return;
    }

    do {
        std::wstring file_name = file_info.cFileName;
        if (file_name == L"." || file_name == L"..") {
            continue;
        }

        if (call_back(file_name, is_dir(ddpath::join(root_path, file_name)))) {
            break;
        }
    } while (::FindNextFile(hFile, &file_info));
    ::FindClose(hFile);
}

void dddir::enum_dir(const std::wstring& dir_path, std::function<bool(const std::wstring& full_path, bool is_dir)> call_back)
{
    if (call_back == nullptr) {
        return;
    }

    enum_dir(dir_path, [&dir_path , &call_back](const std::wstring& sub_path, const std::wstring& name, bool is_dir) {
        return call_back(ddpath::join({ dir_path, sub_path, name}), is_dir);
    });
}

void dddir::enum_dir(const std::wstring& dir_path, std::function<bool(const std::wstring& sub_path, const std::wstring& name, bool is_dir)> call_back)
{
    std::wstring root_path = dir_path;
    ddpath::normal(root_path);
    if (call_back == nullptr || !is_path_exist(root_path) || !is_dir(root_path)) {
        return;
    }

    // 枚举路径
    std::queue<std::wstring> sub_path_queue;
    sub_path_queue.push(L"");
    bool should_break = false;
    while (!sub_path_queue.empty()) {
        if (should_break) {
            break;
        }

        std::wstring sub_path = sub_path_queue.front();
        sub_path_queue.pop();
        std::wstring full_path = ddpath::join(root_path, sub_path);
        WIN32_FIND_DATA file_info{ 0 };
        HANDLE hFile = ::FindFirstFile(ddpath::join(full_path, L"*").c_str(), &file_info);
        if (INVALID_HANDLE_VALUE == hFile) {
            continue;
        }

        do {
            if (should_break) {
                break;
            }

            std::wstring file_name = file_info.cFileName;
            if (file_name == L"." || file_name == L"..") {
                continue;
            }

            if (is_dir(ddpath::join(full_path, file_name))) {
                sub_path_queue.push(ddpath::join(sub_path, file_name));
                should_break = call_back(sub_path, file_name, true);
            } else {
                should_break = call_back(sub_path, file_name, false);
            }
        } while (::FindNextFile(hFile, &file_info));
        ::FindClose(hFile);
    }
}

void dddir::enum_dir(const std::wstring& dir_path, std::vector<std::wstring>& out, std::function<bool(const std::wstring&, bool)> filter)
{
    enum_dir(dir_path, [filter, &out](const std::wstring& path, bool is_dir) {
        if (filter == nullptr || filter(path, is_dir)) {
            out.push_back(path);
        }
        return false;
    });
}

} // namespace NSP_DD
