#include "ddbase/stdafx.h"
#include "ddbase/windows/ddshared_memory.h"

#include "ddbase/windows/ddsecurity.h"
#include "ddbase/ddrandom.h"

namespace NSP_DD {

ddshared_memory::ddshared_memory() { }

ddshared_memory::~ddshared_memory()
{
    close();
}

u8* ddshared_memory::get_buff()
{
    if (m_mapview != nullptr) {
        return (u8*)m_mapview;
    }

    m_mapview = ::MapViewOfFile(m_map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    return (u8*)m_mapview;
}

bool ddshared_memory::init(u32 size, const std::wstring& name)
{
    close();
    m_size = size;
    m_name = name;
    m_map = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    if (m_map != nullptr) {
        u8* buff = get_buff();
        if (buff == nullptr) {
            return false;
        }
        MEMORY_BASIC_INFORMATION info;
        if (::VirtualQuery(buff, &info, sizeof(info)) == 0) {
            return false;
        }

        m_size = (u32)info.RegionSize;
        return true;
    }

    // 以低权限创建
    SECURITY_ATTRIBUTES sa;
    sa.bInheritHandle = FALSE;
    SECURITY_DESCRIPTOR sd;
    sa.lpSecurityDescriptor = (void*)&sd;
    if (!ddsecurity::create_low_sa(sa)) {
        return false;
    }
    m_map = ::CreateFileMappingW(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, (DWORD)size, name.c_str());

    // 如果创建失败的话，尝试创建文件映射
    if (m_map == nullptr) {
        WCHAR tmpPath[MAX_PATH + 1] = { 0 };
        if (0 == ::GetTempPath(MAX_PATH + 1, tmpPath)) {
            return nullptr;
        }

        std::wstring path = tmpPath;
        path += name;
        HANDLE hFile = ::CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, nullptr);
        if (INVALID_HANDLE_VALUE != hFile) {
            m_map = ::CreateFileMapping(hFile, &sa, PAGE_READWRITE, 0, (DWORD)size, name.c_str());
            ::CloseHandle(hFile);
        }
    }

    if (m_map == nullptr) {
        return nullptr;
    }

    return (m_map != nullptr);
}

void ddshared_memory::close()
{
    if (m_mapview != nullptr) {
        (void)::UnmapViewOfFile(m_mapview);
        m_mapview = nullptr;
    }

    if (m_map != nullptr) {
        ::CloseHandle(m_map);
        m_map = NULL;
    }
}

//////////////////////////////ddasync_shared_memory////////////////////////////////////////////
ddasync_shared_memory::ddasync_shared_memory()
{
}

ddasync_shared_memory::~ddasync_shared_memory()
{
    close();
}

bool ddasync_shared_memory::init(u32 size, const std::wstring& name)
{
    if (m_mutex == nullptr) {
        m_mutex = new(std::nothrow) ddprocess_mutex();
        if (m_mutex == nullptr || !m_mutex->init(name)) {
            return false;
        }
    }

    if (!m_mutex->lock()) {
        return false;
    }

    do {
        if (m_shared_memory != nullptr) {
            delete m_shared_memory;
        }

        m_shared_memory = new (std::nothrow) ddshared_memory();
        if (m_shared_memory == nullptr) {
            break;
        }

        if (!m_shared_memory->init(size, name)) {
            break;
        }

        m_mutex->unlock();
        return true;
    } while (0);

    m_mutex->unlock();
    close();
    return false;
}

u8* ddasync_shared_memory::lockbuff(u32 waitTime)
{
    if (m_mutex == nullptr || !m_mutex->lock(waitTime)) {
        return nullptr;
    }

    if (m_shared_memory == nullptr) {
        return nullptr;
    }

    return m_shared_memory->get_buff();
}

void ddasync_shared_memory::unlock()
{
    if (m_mutex != nullptr) {
        m_mutex->unlock();
    }
}

void ddasync_shared_memory::close()
{
    (void)lockbuff();
    if (m_shared_memory != nullptr) {
        delete m_shared_memory;
        m_shared_memory = nullptr;
    }
    unlock();

    if (m_mutex != nullptr) {
        delete m_mutex;
        m_mutex = nullptr;
    }
}
} // namespace NSP_DD
