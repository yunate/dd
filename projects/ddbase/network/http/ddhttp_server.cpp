#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/http/ddhttp_server.h"
#include <mswsock.h>

namespace NSP_DD {
std::shared_ptr<ddhttp_server> ddhttp_server::create_inst(
    ddiocp_with_dispatcher* iocp,
    const ddaddr& listen_addr,
    const std::shared_ptr<ddcert>& cert,
    const ddasync_caller& async_caller /* = nullptr */)
{
    std::shared_ptr<ddhttp_server> inst(new (std::nothrow)ddhttp_server());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    inst->m_acceptor = ddtcp_acceptor::create_inst(iocp, listen_addr);
    if (inst->m_acceptor == nullptr) {
        return nullptr;
    }

    inst->m_async_caller = async_caller;
    inst->m_use_https = cert != nullptr;
    inst->m_cert = cert;
    return inst;
}

ddhttp_server::~ddhttp_server()
{
    close_socket();
}

void ddhttp_server::close_socket()
{
    if (m_acceptor != nullptr) {
        m_acceptor->close_socket();
        m_acceptor.reset();
    }
}

class ddhttp_socket_server_helper : public ddhttp_server_socket
{
public:
    void set_tcp_socket(const std::shared_ptr<ddtcp_socket>& socket)
    {
        m_socket = socket;
    }

    void set_async_caller(const ddasync_caller& async_caller)
    {
        m_async_caller = async_caller;
    }
};

ddcoroutine<std::shared_ptr<ddhttp_server_socket>> ddhttp_server::accept(ddexpire expire /* = ddexpire::never */)
{
    DDASSERT(m_acceptor != nullptr);
    auto tcp_socket = co_await m_acceptor->accept(expire);
    if (tcp_socket == nullptr) {
        co_return nullptr;
    }

    std::shared_ptr<ddhttp_socket_server_helper> client = std::make_shared<ddhttp_socket_server_helper>();
    if (client == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        co_return nullptr;
    }

    client->set_tcp_socket(tcp_socket);

    if (m_use_https) {
        if (!client->init_tls(m_cert) || !co_await client->https_hand_shake(expire)) {
            co_return nullptr;
        }
    }

    client->set_async_caller(m_async_caller);
    co_return client;
}
} // namespace NSP_DD

