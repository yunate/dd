#include "ddbase/stdafx.h"
#include "ddbase/file/ddbig_file_utils.h"
#include "ddbase/ddmath.h"
#include <windows.h>
namespace NSP_DD {

/** 对齐（64K）
*/
#define ALIGN_SIZE 65536

ddbig_file_mapper::ddbig_file_mapper():
    m_hFileMap(nullptr),
    m_bginPos(0),
    m_fileSize(0)
{
}

ddbig_file_mapper::~ddbig_file_mapper()
{
    if (m_hFileMap != nullptr)
    {
        ::CloseHandle(m_hFileMap);
        m_hFileMap = nullptr;
    }
}

bool ddbig_file_mapper::map_file(const std::wstring& file_path)
{
    // 打开文件句柄
    HANDLE hFile = ::CreateFile(file_path.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                nullptr,
                                OPEN_EXISTING,
                                FILE_FLAG_OVERLAPPED,
                                nullptr);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return false;
    }

    LARGE_INTEGER fileSizeLar;

    if (!::GetFileSizeEx(hFile, &fileSizeLar))
    {
        ::CloseHandle(hFile);
        return false;
    }

    // 创建映射
    void* hFileMap = ::CreateFileMapping(hFile,
                                         nullptr,
                                         PAGE_READWRITE,
                                         fileSizeLar.HighPart,
                                         fileSizeLar.LowPart,
                                         nullptr);

    ::CloseHandle(hFile);

    if (hFileMap == nullptr)
    {
        return false;
    }

    if (m_hFileMap != nullptr)
    {
        ::CloseHandle(m_hFileMap);
        m_hFileMap = nullptr;
    }

    m_hFileMap = hFileMap;
    m_fileSize = (s64)fileSizeLar.QuadPart;
    m_bginPos = 0;
    return true;
}

sp_file_block ddbig_file_mapper::get_next_block(s32 sizeRaw/* = g_file_block_default_size */, char align /* = '\n' */)
{
    sp_file_block spFileBlock = get_block(m_bginPos, sizeRaw, align);

    if (spFileBlock != nullptr)
    {
        m_bginPos += spFileBlock->m_blockSize;
    }

    return spFileBlock;
}

sp_file_block ddbig_file_mapper::get_block(s64 beginPos, s32 sizeRaw, char align)
{
    sp_file_block spFileBlock(new(std::nothrow) ddfile_block());

    if (m_hFileMap == nullptr || spFileBlock == nullptr)
    {
        return nullptr;
    }

    if (beginPos >= m_fileSize)
    {
        return nullptr;
    }

    // 这个是映射的大小
    s32 size = sizeRaw;
    if (size <= 0 || size >= 1024 * 1024 * 1024)
    {
        size = g_file_block_default_size;
    }

    // 对齐
    s32 offset = (s32)(beginPos % ALIGN_SIZE);
    beginPos -= offset;
    size += offset;

    if (size + beginPos > m_fileSize)
    {
        size = (s32)(m_fileSize - beginPos);
    }

    char* pFileBlock = (char*)::MapViewOfFile(m_hFileMap,
                                              PAGE_READWRITE,
                                              H64(beginPos),
                                              L64(beginPos),
                                              size);

    if (pFileBlock == nullptr)
    {
        return nullptr;
    }

    if (align == 0)
    {
        spFileBlock->m_pBlock = pFileBlock;
        spFileBlock->m_offSet = offset;
        spFileBlock->m_blockSize = size - offset;
        return spFileBlock;
    }

    s32 tmp = size - 1;
    while (tmp >= offset)
    {
        if (pFileBlock[tmp] == align)
        {
            break;
        }

        --tmp;
    }

    if (tmp == offset - 1)
    {
        // 没有找到对齐字符
        return nullptr;
    }

    // 重新映射
    size = tmp + 1;
    ::UnmapViewOfFile(pFileBlock);
    pFileBlock = nullptr;
    pFileBlock = (char*)::MapViewOfFile(m_hFileMap,
                                        FILE_MAP_READ,
                                        H64(beginPos),
                                        L64(beginPos),
                                        size);
    if (pFileBlock == nullptr)
    {
        // 失败
        return nullptr;
    }

    spFileBlock->m_pBlock = pFileBlock;
    spFileBlock->m_offSet = offset;
    spFileBlock->m_blockSize = size - offset;
    return spFileBlock;
}

ddfile_block::~ddfile_block()
{
    if (m_pBlock != nullptr)
    {
        ::UnmapViewOfFile(m_pBlock);
        m_pBlock = nullptr;
    }
}

} // namespace NSP_DD
