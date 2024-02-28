#ifndef ddbase_network_ddtcp_h_
#define ddbase_network_ddtcp_h_
#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/iocp/ddiocp_io_dispatch.h"
#include "ddbase/network/ddnetwork_utils.h"
#include <memory>
#include <map>

namespace NSP_DD {
class ddtcp_socket : public ddiocp_io_dispatch
{
    DDNO_COPY_MOVE(ddtcp_socket);
public:
    ddtcp_socket() = default;
    virtual ~ddtcp_socket();

    void close_socket();

    HANDLE get_handle() override
    {
        return (HANDLE)m_socket;
    }

    inline SOCKET get_socket()
    {
        return m_socket;
    }

protected:
    SOCKET m_socket = INVALID_SOCKET;
};
} // namespace NSP_DD

namespace NSP_DD {
class ddtcp_acceptor_item;
class ddtcp_acceptor : public ddiiocp_dispatch
{
    DDNO_COPY_MOVE(ddtcp_acceptor);
    ddtcp_acceptor() = default;
public:
    static std::shared_ptr<ddtcp_acceptor> create_inst(ddiocp_with_dispatcher* iocp, const ddaddr& listen_addr);
    ~ddtcp_acceptor();

    void close_socket();
    inline SOCKET get_socket()
    {
        return m_socket;
    }

    // 返回已经绑定到iocp上的ddtcp_socket对象
    // 如果返回nullptr, 使用dderror_code::get_last_error()获得失败具体原因
    // 如果dderror_code::get_last_error() == dderror_code::time_out, 表示超时
    // 返回的std::shared_ptr<ddtcp_socket>的ref count 为1
    ddcoroutine<std::shared_ptr<ddtcp_socket>> accept(ddexpire expire = ddexpire::never);

// from ddiocp_timeout_dispatch
    HANDLE get_handle() final
    {
        return (HANDLE)m_socket;
    }
protected:
    void accept(const std::function<void(std::shared_ptr<ddtcp_socket>)>& callback, ddexpire expire = ddexpire::never);
    void on_iocp_complete_v0(const ddiocp_item& item) final;
    void on_timeout(OVERLAPPED*) final;

private:
    bool init(const ddaddr& listen_addr);
    bool m_ipv4_6 = false;
    SOCKET m_socket = INVALID_SOCKET;
    void* m_lpfn_accecpex = nullptr;

private:
    void add_pending(ddtcp_acceptor_item* item);
    void remove_pending(ddtcp_acceptor_item* item);
    ddtcp_acceptor_item* get_item(OVERLAPPED* ov);
    std::mutex m_mutex;
    std::map<OVERLAPPED*, ddtcp_acceptor_item*> m_pendings;
};
} // namespace NSP_DD

//////////////////////////////////ddtcp_connector//////////////////////////////////
namespace NSP_DD {
class ddsocket_connector_item;
class ddtcp_connector
{
    DDNO_COPY_MOVE(ddtcp_connector);
    ddtcp_connector() = default;
public:
    static std::shared_ptr<ddtcp_connector> create_inst(ddiocp_with_dispatcher* iocp);
    ~ddtcp_connector();
    void close_socket();

    // 返回已经绑定到iocp上的ddtcp_socket对象
    // 如果返回nullptr, 使用dderror_code::get_last_error()获得失败具体原因
    // 如果dderror_code::get_last_error() == dderror_code::time_out, 表示超时
    // 返回的std::shared_ptr<ddtcp_socket>的ref count 为1
    ddcoroutine<std::shared_ptr<ddtcp_socket>> connect_to(const ddaddr& addr, ddexpire expire = ddexpire::never);
    inline ddiocp_with_dispatcher* get_iocp() { return m_iocp; }

private:
    void connect_to(const ddaddr& addr, const std::function<void(const std::shared_ptr<ddtcp_socket>&)>& callback, ddexpire expire = ddexpire::never);
    void add_pending(const std::shared_ptr<ddsocket_connector_item>& item);
    void remove_pending(ddsocket_connector_item* item);

    ddiocp_with_dispatcher* m_iocp = nullptr;
    std::mutex m_mutex;
    std::list<std::shared_ptr<ddsocket_connector_item>> m_pendings;

    friend ddsocket_connector_item;
};
} // namespace NSP_DD
#endif // ddbase_network_ddtcp_h_