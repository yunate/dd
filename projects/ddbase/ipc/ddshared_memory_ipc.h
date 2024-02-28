#ifndef ddbase_ipc_ddshared_memory_ipc_h_
#define ddbase_ipc_ddshared_memory_ipc_h_
#include "ddbase/windows/ddshared_memory.h"
#include "ddbase/ddsingle_limited.h"
#include "ddbase/ddnocopyable.hpp"
namespace NSP_DD {

class ddshared_memory_ipc_base
{
    DDNO_COPY_MOVE(ddshared_memory_ipc_base);
public:
    ddshared_memory_ipc_base() = default;
    virtual ~ddshared_memory_ipc_base();
    virtual bool create(const std::wstring& name, u32 size);
    void close();
protected:
    ddshared_memory* m_shared_memory = nullptr;
    HANDLE m_recv_event = NULL;
    HANDLE m_send_event = NULL;
    u32 m_max_size = 0;
    std::wstring m_name;
};

class ddshared_memory_ipc_server : public ddshared_memory_ipc_base
{
public:
    ddshared_memory_ipc_server() = default;
    ~ddshared_memory_ipc_server() = default;
    virtual bool create(const std::wstring& name, u32 size);
    // time_out 等于 0xFFFFFFFF 时候永不超时
    bool recv(ddbuff& buff, u32 time_out = 0xFFFFFFFF);

    // 获取读event，以便iocp
    HANDLE get_recv_event();

private:
    // 直接读取到buff中，暴露以便iocp
    bool read(ddbuff& buff);
private:
    ddsingle_limited m_single_limited;
};

class ddshared_memory_ipc_client : public ddshared_memory_ipc_base
{
public:
    ddshared_memory_ipc_client() = default;
    ~ddshared_memory_ipc_client() = default;
    // 如果size大于共享内存大小则返回false，由调用者切片
    // 如果size大于buff大小则crash
    bool send(const void* buff, u32 size, u32 time_out = 0xFFFFFFFF);
};
} // namespace NSP_DD
#endif // ddbase_ipc_ddshared_memory_ipc_h_
