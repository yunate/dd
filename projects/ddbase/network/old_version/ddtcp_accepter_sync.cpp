#include "ddbase/stdafx.h"
#include "ddbase/network/ddtcp_accepter_sync.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/dderror_code.h"

#include <memory>
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {

class ddsocket_sync_friend_helper : public ddsocket_sync
{
    friend class ddtcp_accepter_sync_impl;
};

static SOCKET create_sync_socket(bool ipv4_6)
{
    if (ipv4_6) {
        return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else {
        return ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }
}

class ddtcp_accepter_sync_impl : public ddsocket_sync, public ddtcp_accepter_sync
{
    DDNO_COPY_MOVE(ddtcp_accepter_sync_impl);

    ddtcp_accepter_sync_impl() = default;
public:
    ~ddtcp_accepter_sync_impl() = default;
    static std::unique_ptr<ddtcp_accepter_sync> create_instance(const ddaddr& addr)
    {
        auto accepter = std::unique_ptr<ddtcp_accepter_sync_impl>(new (std::nothrow)ddtcp_accepter_sync_impl());
        if (accepter == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            return nullptr;
        }

        accepter->m_socket = create_sync_socket(addr.ipv4_6);
        if (accepter->m_socket == INVALID_SOCKET) {
            return nullptr;
        }

        // bind and listen
        if (!ddnet_utils::bind(accepter->m_socket, addr)) {
            return nullptr;
        }

        if (::listen(accepter->get_socket(), SOMAXCONN) == SOCKET_ERROR) {
            return nullptr;
        }

        return accepter;
    }

    std::unique_ptr<ddsocket_sync> accept() override
    {
        auto socket = std::unique_ptr<ddsocket_sync_friend_helper>(new (std::nothrow)ddsocket_sync_friend_helper());
        if (socket == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            return nullptr;
        }

        socket->m_socket = ::accept(m_socket, NULL, NULL);
        if (socket->m_socket == INVALID_SOCKET) {
            return nullptr;
        }

        // set socket options so that we can retrieve the ip and port
        (void)::setsockopt(socket->m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(SOCKET));
        return socket;
    }
};

/////////////////////////////////////////ddtcp_connector_sync/////////////////////////////////////////
std::unique_ptr<ddtcp_accepter_sync> ddtcp_accepter_sync::create_instance(const ddaddr& addr)
{
     return ddtcp_accepter_sync_impl::create_instance(addr);
}

} // namespace NSP_DD
