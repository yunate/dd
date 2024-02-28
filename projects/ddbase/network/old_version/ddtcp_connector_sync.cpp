#include "ddbase/stdafx.h"
#include "ddbase/network/ddtcp_connector_sync.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/dderror_code.h"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
/////////////////////////////////////////ddtcp_connector_sync_item/////////////////////////////////////////
static SOCKET create_sync_socket(bool ipv4_6)
{
    if (ipv4_6) {
        return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else {
        return ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }
}

class ddtcp_connector_sync_item : public ddsocket_sync
{
    DDNO_COPY_MOVE(ddtcp_connector_sync_item);
    ddtcp_connector_sync_item() = default;

public:
    ~ddtcp_connector_sync_item() = default;
    static ddtcp_connector_sync_item* create_tcp_connector_item(bool ipv4_6)
    {
        auto connector = std::unique_ptr<ddtcp_connector_sync_item>(new (std::nothrow)ddtcp_connector_sync_item());
        if (connector == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            return nullptr;
        }

        connector->m_ipv4_6 = ipv4_6;
        connector->m_socket = create_sync_socket(ipv4_6);
        if (connector->m_socket == INVALID_SOCKET) {
            return nullptr;
        }

        return connector.release();
    }

    bool connect(const ddaddr& addr)
    {
        if (addr.ipv4_6 != m_ipv4_6) {
            dderror_code::set_last_error(dderror_code::param_mismatch);
            return false;
        }

        ::sockaddr* server_addr = nullptr;
        ::sockaddr_in server_addr4{};
        ::sockaddr_in6 server_addr6{};
        int addr_len = 0;
        if (m_ipv4_6) {
            server_addr = (sockaddr*)&server_addr4;
            addr_len = sizeof(sockaddr_in);
        } else {
            server_addr = (sockaddr*)&server_addr6;
            addr_len = sizeof(sockaddr_in6);
        }

        if (!ddnet_utils::ddaddr_to_sockaddr(addr, server_addr)) {
            return false;
        }

        if (::connect(m_socket, server_addr, addr_len) != 0) {
            return false;
        }

        // set socket options so that we can retrieve the ip and port
        (void)::setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
        return true;
    }

private:
    bool m_ipv4_6 = true;
};

/////////////////////////////////////////ddtcp_connector_sync_impl/////////////////////////////////////////
class ddtcp_connector_sync_impl : public ddtcp_connector_sync
{
    friend ddtcp_connector_sync;
public:
    std::unique_ptr<ddsocket_sync> connect(const ddaddr& addr) override
    {
        ddtcp_connector_sync_item* connector = ddtcp_connector_sync_item::create_tcp_connector_item(m_ipv4_6);
        if (connector == nullptr) {
            return nullptr;
        }

        if (!connector->connect(addr)) {
            delete connector;
            return nullptr;
        }

        return std::unique_ptr<ddsocket_sync>(connector);
    }

private:
    bool m_ipv4_6 = true;
};

/////////////////////////////////////////ddtcp_connector_sync/////////////////////////////////////////
std::unique_ptr<ddtcp_connector_sync> ddtcp_connector_sync::create_instance(bool ipv4_6)
{
    ddtcp_connector_sync_impl* impl = new (std::nothrow)ddtcp_connector_sync_impl();
    if (impl == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }
    impl->m_ipv4_6 = ipv4_6;
    return std::unique_ptr<ddtcp_connector_sync>(impl);
}

} // namespace NSP_DD
