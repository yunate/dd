#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/http/ddhttp_server.h"
#include <mswsock.h>

namespace NSP_DD {
std::shared_ptr<ddhttp_server> ddhttp_server::create_inst(
    ddiocp_with_dispatcher* iocp,
    const ddaddr& listen_addr,
    const std::shared_ptr<ddcert>& cert)
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

ddcoroutine<std::shared_ptr<ddhttp_server_socket>> ddhttp_server::accept(ddexpire expire /* = ddexpire::never */)
{
    DDASSERT(m_acceptor != nullptr);
    auto tcp_socket = co_await m_acceptor->accept(expire);
    if (tcp_socket == nullptr) {
        co_return nullptr;
    }

    std::shared_ptr<ddhttp_server_socket> client = std::make_shared<ddhttp_server_socket>();
    if (client == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        co_return nullptr;
    }

    if (!co_await client->init(tcp_socket, m_cert, expire)) {
        co_return nullptr;
    }

    co_return client;
}
} // namespace NSP_DD

