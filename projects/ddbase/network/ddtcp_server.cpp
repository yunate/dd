#include "ddbase/stdafx.h"
#include "ddbase/network/ddtcp_server.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/dderror_code.h"

#include <map>
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {

class ddtcp_server_impl : public ddtcp_server
{
    DDNO_COPY_MOVE(ddtcp_server_impl);
public:
    ddtcp_server_impl() = default;
    ~ddtcp_server_impl()
    {
        m_accepters.clear();
        m_iocp = nullptr;
    }

    bool init(ddiocp_with_dispatcher& iocp, const std::function<void(std::shared_ptr<ddsocket_async>)>& client_connect_callback)
    {
        if (client_connect_callback == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            return false;
        }
        m_iocp = &iocp;
        m_client_connect_callback = client_connect_callback;
        return true;
    }

    void listen_port(u16 port, bool ipv4_6, const std::function<void()>& on_error) override
    {
        DDASSERT(m_iocp != nullptr);
        m_iocp->run_in_iocp_thread_unsafe([this, port, ipv4_6, on_error]() {
            if (m_accepters.find(port) != m_accepters.end()) {
                dderror_code::set_last_error(dderror_code::key_exist);
                on_error();
                return;
            }

            auto accepter = ddtcp_accepter_async::create_instance(ddaddr{ ddnet_utils::ip_anys(ipv4_6), port, ipv4_6 }, *m_iocp);
            if (accepter == nullptr) {
                on_error();
                return;
            }

            if (accept(accepter.get(), port, on_error)) {
                m_accepters[port] = accepter;
            }
        });
    }

    void unlisten_port(u16 port) override
    {
        m_iocp->run_in_iocp_thread_unsafe([this, port]() {
            if (m_accepters.find(port) == m_accepters.end()) {
                dderror_code::set_last_error(dderror_code::key_not_find);
                return;
            }

            m_accepters.erase(port);
        });
    }

private:
    bool accept(ddtcp_accepter_async* accepter, u16 port, const std::function<void()>& on_error)
    {
        DDASSERT(accepter != nullptr);
        if (!accepter->accept_async([this, accepter, port, on_error](std::shared_ptr<ddsocket_async> socket) {
            this->on_accept(std::move(socket), accepter, port, on_error);
        })) {
            on_error();
            unlisten_port(port);
            return false;
        }
        return true;
    }

    void on_accept(std::shared_ptr<ddsocket_async> socket, ddtcp_accepter_async* accepter, u16 port, const std::function<void()>& on_error)
    {
        // We do not think the accept socker raise an error.
        // One possible scenario for socket == nullptr is, the client immediately closes the socket after connecting
        //if (socket == nullptr) {
        //    on_error();
        //    unlisten_port(port);
        //    return;
        //}

        m_client_connect_callback(socket);

        // continue accept
        (void)accept(accepter, port, on_error);
    }

    ddiocp_with_dispatcher* m_iocp = nullptr;
    std::map<u16, std::shared_ptr<ddtcp_accepter_async>> m_accepters;
    std::function<void(std::shared_ptr<ddsocket_async>)> m_client_connect_callback;
};

/////////////////////////////////////ddtcp_server/////////////////////////////////////
std::unique_ptr<ddtcp_server> ddtcp_server::create_instance(ddiocp_with_dispatcher& iocp, const std::function<void(std::shared_ptr<ddsocket_async>)>& client_connect_callback)
{
    std::unique_ptr<ddtcp_server_impl> impl = std::make_unique<ddtcp_server_impl>();
    if (!impl->init(iocp, client_connect_callback)) {
        return nullptr;
    }
    return impl;
}
} // namespace NSP_DD
