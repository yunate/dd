#include "ddbase/stdafx.h"
#include "ddbase/network/ddtcp_accepter_async.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/dderror_code.h"

#include <map>
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
/////////////////////////////////////////ddtcp_accepter_async_impl/////////////////////////////////////////
class ddsocket_async_friend_helper : public ddsocket_async
{
    friend class ddtcp_accepter_async_impl;
};

static SOCKET create_async_socket(bool ipv4_6)
{
    if (ipv4_6) {
        return ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    } else {
        return ::WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    }
}

class ddtcp_accepter_async_impl : public ddtcp_accepter_async, public ddsocket, public ddiiocp_dispatch
{
    DDNO_COPY_MOVE(ddtcp_accepter_async_impl);

    ddtcp_accepter_async_impl() = default;
public:
    static std::shared_ptr<ddtcp_accepter_async> create_instance(const ddaddr& addr, ddiocp_with_dispatcher& iocp)
    {
        std::shared_ptr<ddtcp_accepter_async_impl> accepter(new(std::nothrow) ddtcp_accepter_async_impl());
        if (accepter == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            return nullptr;
        }

        accepter->m_ipv4_6 = addr.ipv4_6;
        accepter->m_socket = create_async_socket(addr.ipv4_6);
        if (accepter->m_socket == INVALID_SOCKET) {
            return false;
        }

        // 获取AcceptEx函数指针
        GUID GuidAcceptEx = WSAID_ACCEPTEX;
        DWORD dwBytes = 0;
        if (::WSAIoctl(accepter->get_socket(), SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
            &accepter->m_lpfn_accecpex, sizeof(accepter->m_lpfn_accecpex), &dwBytes, NULL, NULL) == SOCKET_ERROR) {
            return nullptr;
        }

        // bind and listen
        if (!ddnet_utils::bind(accepter->m_socket, addr)) {
            return nullptr;
        }

        if (::listen(accepter->get_socket(), SOMAXCONN) == SOCKET_ERROR) {
            return nullptr;
        }

        // bind to iocp
        if (!iocp.watch(accepter)) {
            return nullptr;
        }

        return accepter;
    }

    ~ddtcp_accepter_async_impl()
    {
        dderror_code_guard guard;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            for (auto& it : m_pending_clients) {
                delete it.second;
            }
            m_pending_clients.clear();
        }
    }

    bool accept_async(const std::function<void(std::shared_ptr<ddsocket_async>)>& callback, u64 timeout /*= INFINITE*/) override
    {
        if (m_socket == INVALID_SOCKET) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        ddtcp_ov_item* ovi = nullptr;
        bool success = false;
        ddexec_guard guard([&]() {
            if (success) {
                return;
            }

            if (ovi != nullptr) {
                std::lock_guard<std::recursive_mutex> locker(m_mutex);
                m_pending_clients.erase(&ovi->ov);
                remove_timeout(&ovi->ov);
                delete ovi;
                ovi = nullptr;
            }
        });

        ovi = new(std::nothrow) ddtcp_ov_item();
        if (ovi == nullptr || !ovi->init(m_ipv4_6)) {
            return false;
        }
        ovi->callback = callback;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            m_pending_clients.insert({ &ovi->ov, ovi });
        }
        set_timeout(&ovi->ov, timeout);

        DWORD len = 0;
        bool accept_result = false;
        if (m_ipv4_6) {
            accept_result = ((LPFN_ACCEPTEX)m_lpfn_accecpex)(get_socket(), ovi->socket->get_socket(), ovi->buff.data(), 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &len, &ovi->ov);
        } else {
            accept_result = ((LPFN_ACCEPTEX)m_lpfn_accecpex)(get_socket(), ovi->socket->get_socket(), ovi->buff.data(), 0, sizeof(sockaddr_in6) + 16, sizeof(sockaddr_in6) + 16, &len, &ovi->ov);
        }

        if (!accept_result && ::WSAGetLastError() != ERROR_IO_PENDING) {
            return false;
        }

        success = true;
        return true;
    }

    // accept 并 接受第一个报文
    // @param[in] callback:
    //      @param[in] first_pkg_buff 接受的第一个报文, 报文最大限制为1024, 可能会发生粘包
    // bool accept_with_first_pkg_async(const std::function<void(ddsocket_async*, const std::vector<u8>& first_pkg_buff)>& callback, u64 timeout = INFINITE)
    // {
    //    return accept_async([callback](ddsocket_async* socket){
    //        if (socket == nullptr) {
    //            callback(nullptr, std::vector<u8>());
    //            return;
    //        }
    //
    //        std::shared_ptr<std::vector<u8>> buff(new(std::nothrow) std::vector<u8>(1024));
    //        if (buff == nullptr) {
    //            dderror_code::set_last_error(dderror_code::out_of_memory);
    //            callback(nullptr, std::vector<u8>());
    //            delete socket;
    //            return;
    //        }
    //
    //        if (!socket->recv((*buff).data(), (*buff).size(), [buff, callback, socket](bool successful, u64 recved) {
    //            if (successful) {
    //                (*buff).resize(recved);
    //                callback(socket, *buff);
    //            } else {
    //                delete socket;
    //            }
    //        })) {
    //            delete socket;
    //        }
    //    }, timeout);
    //}

    virtual HANDLE get_handle() override
    {
        return (HANDLE)get_socket();
    }
protected:
    // ddiiocp_dispatch
    virtual void on_iocp_complete(const ddiocp_item& item)
    {
        m_mutex.lock();
        ddtcp_ov_item* ov_item = m_pending_clients[item.overlapped];
        m_pending_clients.erase(item.overlapped);
        m_mutex.unlock();

        DDASSERT(ov_item != nullptr);
        if (ov_item->callback) {
            if (item.has_error) {
                ov_item->callback(nullptr);
            } else {
                // 这儿iocp不会为空,因为该函数在本类中调用的所以本类还没有被释放
                // on_iocp_complete函数是在io线程中跑的,所以iocp也没有被释放.
                auto iocp = get_iocp();
                if (iocp != nullptr && iocp->watch(ov_item->socket)) {
                    // set socket options so that we can retrieve the ip and port
                    (void)::setsockopt(ov_item->socket->get_socket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(SOCKET));
                    ov_item->callback(std::shared_ptr<ddsocket_async>(ov_item->socket));
                    ov_item->socket = nullptr;
                } else {
                    ov_item->callback(nullptr);
                }
            }
        }
        delete ov_item;
    }

    struct ddtcp_ov_item
    {
        ~ddtcp_ov_item()
        {
            dderror_code_guard guard;
            if (ov.hEvent != NULL) {
                ::WSACloseEvent(ov.hEvent);
                ov.hEvent = NULL;
            }
        }
        bool init(bool ipv4_6)
        {
            std::shared_ptr<ddsocket_async_friend_helper> socket_helper(new(std::nothrow) ddsocket_async_friend_helper());
            if (socket_helper == nullptr) {
                dderror_code::set_last_error(dderror_code::out_of_memory);
                return false;
            }

            socket_helper->m_socket = create_async_socket(ipv4_6);
            if (socket_helper->m_socket == INVALID_SOCKET) {
                return false;
            }

            socket = socket_helper;

            if (ipv4_6) {
                buff.resize(sizeof(sockaddr_in) + 16 + sizeof(sockaddr_in) + 16);
            } else {
                buff.resize(sizeof(sockaddr_in6) + 16 + sizeof(sockaddr_in6) + 16);
            }

            ov.hEvent = ::WSACreateEvent();
            if (ov.hEvent == NULL) {
                return false;
            }

            return true;
        }

        OVERLAPPED ov{ 0 };
        std::shared_ptr<ddsocket_async> socket = nullptr;
        std::vector<char> buff;
        std::function<void(std::shared_ptr<ddsocket_async>)> callback;
    };
    bool m_ipv4_6 = true;
    std::map<OVERLAPPED*, ddtcp_ov_item*> m_pending_clients;
    void* m_lpfn_accecpex = NULL;
};

/////////////////////////////////////////ddtcp_accepter_async/////////////////////////////////////////
std::shared_ptr<ddtcp_accepter_async> ddtcp_accepter_async::create_instance(const ddaddr& addr, ddiocp_with_dispatcher& iocp)
{
    return ddtcp_accepter_async_impl::create_instance(addr, iocp);
}

} // namespace NSP_DD
