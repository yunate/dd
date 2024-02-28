
#include "ddbase/stdafx.h"
#include "ddbase/ipc/ddshared_memory_ipc.h"
namespace NSP_DD {

ddshared_memory_ipc_base::~ddshared_memory_ipc_base()
{
    close();
}

bool ddshared_memory_ipc_base::create(const std::wstring& name, u32 size)
{
    m_name = name;
    do {
        if (m_shared_memory != nullptr) {
            delete m_shared_memory;
        }

        m_shared_memory = new(std::nothrow) ddshared_memory();
        if (m_shared_memory == nullptr || !m_shared_memory->init(size, name)) {
            break;
        }

        if (m_shared_memory->get_buff() == nullptr) {
            break;
        }

        std::wstring recv_event_name = name + L"_shared_memory_ipc_recv_event";
        m_recv_event = ::CreateEvent(NULL, false, false, recv_event_name.c_str());
        if (m_recv_event == NULL) {
            break;
        }

        std::wstring send_event_name = name + L"_shared_memory_ipc_send_event";
        m_send_event = ::CreateEvent(NULL, false, true, send_event_name.c_str());
        if (m_send_event == NULL) {
            break;
        }

        m_max_size = size;
        return true;
    } while (0);
    
    close();
    return false;
}

void ddshared_memory_ipc_base::close()
{
    if (m_recv_event != NULL) {
        ::CloseHandle(m_recv_event);
        m_recv_event = NULL;
    }

    if (m_send_event != NULL) {
        ::CloseHandle(m_send_event);
        m_send_event = NULL;
    }

    if (m_shared_memory != nullptr) {
        delete m_shared_memory;
        m_shared_memory = nullptr;
    }
}

bool ddshared_memory_ipc_server::create(const std::wstring& name, u32 size)
{
    if (m_single_limited.try_hold_mutex(name + L"_shared_memory_ipc_sever_limited")) {
        return ddshared_memory_ipc_base::create(name, size);
    }
    
    return false;
}

bool ddshared_memory_ipc_server::recv(ddbuff& buff, u32 time_out /*= 0xFFFFFFFF*/)
{
    if (m_shared_memory == nullptr) {
        return false;
    }

    DWORD hr = ::WaitForSingleObject(m_recv_event, time_out);
    if (hr != WAIT_OBJECT_0) {
        return false;
    }

    return read(buff);
}

bool ddshared_memory_ipc_server::read(ddbuff& buff)
{
    char* mem_buff = (char*)m_shared_memory->get_buff();
    if (mem_buff == nullptr) {
        return false;
    }

    u32 size = *((u32*)mem_buff);
    if (size + sizeof(u32) > m_max_size) {
        return false;
    }

    buff.resize(size);
    (void)::memcpy_s(buff.data(), size, (mem_buff + sizeof(u32)), size);
    (void)::SetEvent(m_send_event);
    return true;
}

HANDLE ddshared_memory_ipc_server::get_recv_event()
{
    return m_recv_event;
}

bool ddshared_memory_ipc_client::send(const void* buff, u32 size, u32 time_out /*= 0xFFFFFFFF*/)
{
    DWORD hr = ::WaitForSingleObject(m_send_event, time_out);
    if (hr != WAIT_OBJECT_0) {
        return false;
    }

    char* mem_buff = (char*)m_shared_memory->get_buff();
    if (mem_buff == nullptr) {
        return false;
    }

    if (size + sizeof(u32) > m_max_size) {
        return false;
    }

    *((u32*)mem_buff) = size;
    (void)::memcpy_s((mem_buff + sizeof(u32)), size, buff, size);
    (void)::SetEvent(m_recv_event);
    return true;
}

} // namespace NSP_DD
