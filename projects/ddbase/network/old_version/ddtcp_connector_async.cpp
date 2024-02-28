#include "ddbase/stdafx.h"
#include "ddbase/network/ddtcp_connector_async.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/dderror_code.h"

#include <map>
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
/////////////////////////////////////////ddtcp_connector_async_item/////////////////////////////////////////
static SOCKET create_async_socket(bool ipv4_6)
{
    if (ipv4_6) {
        return ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    } else {
        return ::WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    }
}

class ddtcp_connector_async_item_impl : public ddtcp_connector_async_item
{
    DDNO_COPY_MOVE(ddtcp_connector_async_item_impl);
    ddtcp_connector_async_item_impl() = default;

public:
    ~ddtcp_connector_async_item_impl()
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (m_ov.hEvent != NULL) {
            dderror_code_guard guard;
            ::WSACloseEvent(m_ov.hEvent);
            m_ov.hEvent = NULL;
        }
    }
    static std::shared_ptr<ddtcp_connector_async_item> create_instance(bool ipv4_6, ddiocp_with_dispatcher& iocp)
    {
        std::shared_ptr<ddtcp_connector_async_item_impl> connector_item(new(std::nothrow) ddtcp_connector_async_item_impl());
        if (connector_item == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            return nullptr;
        }

        connector_item->m_ipv4_6 = ipv4_6;
        connector_item->m_socket = create_async_socket(ipv4_6);
        if (connector_item->m_socket == INVALID_SOCKET) {
            return nullptr;
        }

        DWORD dummy = 0;
        ::GUID guid = WSAID_CONNECTEX;
        (void)::WSAIoctl(connector_item->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid, sizeof(GUID),
            &connector_item->m_lpfn_connectex, sizeof(connector_item->m_lpfn_connectex),
            &dummy, NULL, NULL);
        if (connector_item->m_lpfn_connectex == NULL) {
            return nullptr;
        }

        if (!ddnet_utils::bind(connector_item->m_socket, ddaddr { ddnet_utils::ip_anys(ipv4_6), 0, ipv4_6 })) {
            return nullptr;
        }

        connector_item->m_ov.hEvent = ::WSACreateEvent();
        if (connector_item->m_ov.hEvent == NULL) {
            return nullptr;
        }

        if (!iocp.watch(connector_item)) {
            return nullptr;
        }
        return connector_item;
    }

    bool connect(const ddaddr& addr, const std::function<void(bool)>& callback, u64 timeout) override
    {
        if (m_lpfn_connectex == nullptr) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        DDASSERT(callback != nullptr);
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (addr.ipv4_6 != m_ipv4_6) {
            dderror_code::set_last_error(dderror_code::param_mismatch);
            return false;
        }

        if (m_connect_callback != nullptr) {
            dderror_code::set_last_error(dderror_code::operate_pending);
            return false;
        }

        if (m_connected) {
            dderror_code::set_last_error(dderror_code::init_repeat);
            return false;
        }

        DDASSERT(m_socket != INVALID_SOCKET);
        ::sockaddr* connect_addr = nullptr;
        ::sockaddr_in connect_addr4{};
        ::sockaddr_in6 connect_addr6{};
        int addr_len = 0;
        if (m_ipv4_6) {
            connect_addr = (sockaddr*)&connect_addr4;
            addr_len = sizeof(sockaddr_in);
        } else {
            connect_addr = (sockaddr*)&connect_addr6;
            addr_len = sizeof(sockaddr_in6);
        }

        if (!ddnet_utils::ddaddr_to_sockaddr(addr, connect_addr)) {
            return false;
        }

        m_connect_callback = callback;
        set_timeout(&m_ov, timeout);
        DWORD dummy = 0;
        if (!((LPFN_CONNECTEX(m_lpfn_connectex))(m_socket, connect_addr, addr_len, NULL, 0, &dummy, &m_ov)) &&
            ::WSAGetLastError() != ERROR_IO_PENDING) {
            remove_timeout(&m_ov);
            return false;
        }

        return true;
    }

protected:
    // ddiiocp_dispatch
    virtual void on_iocp_complete(const ddiocp_item& item) override
    {
        std::function<void(bool)> connect_callback;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            connect_callback = std::move(m_connect_callback);
        }

        if (connect_callback != nullptr) {
            if (!item.has_error) {
                // set socket options so that we can retrieve the ip and port
                (void)::setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
                m_connected = true;
            }
            connect_callback(!item.has_error);
            return;
        }
        return ddsocket_async::on_iocp_complete(item);
    }

private:
    std::function<void(bool)> m_connect_callback;
    void* m_lpfn_connectex = nullptr;
    OVERLAPPED m_ov{ 0 };
    bool m_connected = false;
    bool m_ipv4_6 = true;
};

std::shared_ptr<ddtcp_connector_async_item> ddtcp_connector_async_item::create_instance(bool ipv4_6, ddiocp_with_dispatcher& iocp)
{
    return ddtcp_connector_async_item_impl::create_instance(ipv4_6, iocp);
}

/////////////////////////////////////////ddtcp_connector_async_impl/////////////////////////////////////////
class ddtcp_connector_async_impl : public ddtcp_connector_async
{
    DDNO_COPY_MOVE(ddtcp_connector_async_impl);
public:
    static std::unique_ptr<ddtcp_connector_async> create_instance(bool ipv4_6, ddiocp_with_dispatcher& iocp)
    {
        ddtcp_connector_async_impl* impl = new(std::nothrow) ddtcp_connector_async_impl();
        if (impl == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            return nullptr;
        }

        impl->m_ipv4_6 = ipv4_6;
        impl->m_iocp = &iocp;
        return std::unique_ptr<ddtcp_connector_async>(impl);
    }

    ddtcp_connector_async_impl() = default;
    ~ddtcp_connector_async_impl()
    {
        m_pending_connector_items.clear();
        m_iocp = nullptr;
    }

    bool connect(const ddaddr& addr, const std::function<void(std::shared_ptr<ddsocket_async>)>& callback, u64 timeout) override
    {
        auto connector_item = ddtcp_connector_async_item::create_instance(m_ipv4_6, *m_iocp);
        if (connector_item == nullptr) {
            return false;
        }

        u64 key = get_next_key();
        m_pending_connector_items[key] = connector_item;
        if (!connector_item->connect(addr, [callback, key, this](bool successful) {
            auto it = m_pending_connector_items[key];
            m_pending_connector_items.erase(key);
            if (successful) {
                callback(std::shared_ptr<ddsocket_async>(it));
            } else {
                callback(nullptr);
            }
        }, timeout)) {
            m_pending_connector_items.erase(key);
            return false;
        }

        return true;
    }

private:
    u64 get_next_key()
    {
        return m_key++;
    }

    std::atomic<u64> m_key = 0;
    std::map<u64, std::shared_ptr<ddtcp_connector_async_item>> m_pending_connector_items;
    bool m_ipv4_6 = true;
    ddiocp_with_dispatcher* m_iocp = nullptr;
};

/////////////////////////////////////////ddtcp_connector_async/////////////////////////////////////////
std::unique_ptr<ddtcp_connector_async> ddtcp_connector_async::create_instance(bool ipv4_6, ddiocp_with_dispatcher& iocp)
{
    return ddtcp_connector_async_impl::create_instance(ipv4_6, iocp);
}

} // namespace NSP_DD
