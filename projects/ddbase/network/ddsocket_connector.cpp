#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/ddsocket_connector.h"
#include "ddbase/ddexec_guard.hpp"

#include <mswsock.h>

namespace NSP_DD {
static SOCKET create_async_socket(bool ipv4_6)
{
    if (ipv4_6) {
        return ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    } else {
        return ::WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    }
}

struct ddsocket_connector_item : public ddiocp_timeout_dispatch
{
public:
    ~ddsocket_connector_item()
    {
        if (socket != INVALID_SOCKET) {
            auto* iocp = get_iocp();
            if (iocp != nullptr) {
                iocp->unwatch(this);
            }

            ::closesocket(socket);
            socket = INVALID_SOCKET;
        }

        if (ov.hEvent != NULL) {
            ::WSACloseEvent(ov.hEvent);
            ov.hEvent = NULL;
        }
    }

    void on_timeout(OVERLAPPED*) final
    {
        ddexec_guard guard([this]() {
            connector->remove_pending(this);
        });

        dderror_code::set_last_error(dderror_code::time_out);
        callback(nullptr);
        connector->remove_pending(this);
        get_iocp()->unwatch(this);
    }

    void on_iocp_complete_v1(const ddiocp_item& item) final
    {
        ddexec_guard guard([this]() {
            connector->remove_pending(this);
        });

        get_iocp()->unwatch(this);
        std::unique_ptr<ddsocket_connector_item> release_guard(this);
        if (item.has_error) {
            return;
        }

        std::unique_ptr<ddsocket> inst = std::make_unique<ddsocket>();
        if (inst == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            callback(nullptr);
            return;
        }

        ddsocket_connector::set_socket(*inst.get(), socket);
        if (!get_iocp()->watch(inst.get())) {
            ddsocket_connector::set_socket(*inst.get(), INVALID_SOCKET);
            callback(nullptr);
            return;
        } else {
            // set socket options so that we can retrieve the ip and port
            (void)::setsockopt(socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
            socket = INVALID_SOCKET;
            callback(std::move(inst));
        }
    }
    inline HANDLE get_handle() override
    {
        return (HANDLE)socket;
    }

    ::OVERLAPPED ov;
    SOCKET socket = INVALID_SOCKET;
    std::function<void(std::unique_ptr<ddsocket> socket)> callback;
    ddsocket_connector* connector = nullptr;
};

void ddsocket_connector::set_socket(ddsocket& dst, SOCKET socket)
{
    dst.m_socket = socket;
}

ddsocket_connector::~ddsocket_connector()
{
    for (auto it : m_pendings) {
        delete it;
    }
}

void ddsocket_connector::add_pending(ddsocket_connector_item* item)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    DDASSERT(item != nullptr);
    m_pendings.push_back(item);
}

void ddsocket_connector::remove_pending(ddsocket_connector_item* item)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    DDASSERT(item != nullptr);
    m_pendings.remove(item);
    delete item;
}

void ddsocket_connector::connect_to(const ddaddr& addr, ddiocp_with_dispatcher* iocp, const std::function<void(std::unique_ptr<ddsocket> socket)>& callback, u64 timeout)
{
    ddsocket_connector_item* item = nullptr;
    do {
        item = new(std::nothrow)ddsocket_connector_item();
        if (item == nullptr) {
            break;
        }

        item->socket = create_async_socket(addr.ipv4_6);
        if (item->socket == INVALID_SOCKET) {
            break;
        }

        DWORD dummy = 0;
        void* lpfn_connectex = nullptr;
        ::GUID guid = WSAID_CONNECTEX;
        (void)::WSAIoctl(item->socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid, sizeof(GUID),
            &lpfn_connectex, sizeof(lpfn_connectex),
            &dummy, NULL, NULL);
        if (lpfn_connectex == NULL) {
            break;
        }

        if (!ddnetwork_utils::bind(item->socket, ddaddr{ ddnetwork_utils::get_zero_ip(addr.ipv4_6), 0, addr.ipv4_6 })) {
            break;
        }

        item->ov.hEvent = ::WSACreateEvent();
        if (item->ov.hEvent == NULL) {
            break;
        }

        item->set_timeout(&item->ov, timeout);
        if (!iocp->watch(item)) {
            break;
        }

        ::sockaddr* connect_addr = nullptr;
        ::sockaddr_in connect_addr4{};
        ::sockaddr_in6 connect_addr6{};
        int addr_len = 0;
        if (addr.ipv4_6) {
            connect_addr = (sockaddr*)&connect_addr4;
            addr_len = sizeof(sockaddr_in);
        } else {
            connect_addr = (sockaddr*)&connect_addr6;
            addr_len = sizeof(sockaddr_in6);
        }

        if (!addr.to_sockaddr(connect_addr)) {
            break;
        }

        DWORD dummy = 0;
        if (!((LPFN_CONNECTEX(lpfn_connectex))(item->socket, connect_addr, addr_len, NULL, 0, &dummy, &item->ov)) &&
            ::WSAGetLastError() != ERROR_IO_PENDING) {
            item->set_timeout(&item->ov);
            break;
        }

        item->callback = callback;
        item->connector = this;
        add_pending(item);
        return;
    } while (0);

    if (item != nullptr) {
        delete item;
    }

    if (callback != nullptr) {
        callback(nullptr);
    }
}

} // namespace NSP_DD