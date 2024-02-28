#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/ddtcp.h"
#include "ddbase/ddexec_guard.hpp"

#include <mswsock.h>

namespace NSP_DD {
ddtcp_socket::~ddtcp_socket()
{
    close_socket();
}

void ddtcp_socket::close_socket()
{
    if (m_socket != INVALID_SOCKET) {
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

class ddsocket_friend : public ddtcp_socket
{
public:
    ~ddsocket_friend()
    {
    }
    void set_socket(SOCKET socket)
    {
        m_socket = socket;
    }
};
} // namespace NSP_DD

//////////////////////////////////ddtcp_acceptor//////////////////////////////////
namespace NSP_DD {
class ddtcp_acceptor_item
{
public:
    ~ddtcp_acceptor_item()
    {
        dderror_code_guard guard;
        if (m_listen_ov.hEvent != NULL) {
            ::WSACloseEvent(m_listen_ov.hEvent);
            m_listen_ov.hEvent = NULL;
        }

        if (m_socket != INVALID_SOCKET) {
            ::closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }
    }

    bool init()
    {
        m_socket = ddnetwork_utils::create_async_tcp_socket(m_ipv4_6);
        if (m_socket == INVALID_SOCKET) {
            return false;
        }

        if (m_ipv4_6) {
            buff.resize(sizeof(sockaddr_in) + 16 + sizeof(sockaddr_in) + 16);
        } else {
            buff.resize(sizeof(sockaddr_in6) + 16 + sizeof(sockaddr_in6) + 16);
        }

        m_listen_ov.hEvent = ::WSACreateEvent();
        if (m_listen_ov.hEvent == NULL) {
            return false;
        }

        return true;
    }

    inline OVERLAPPED* get_ov()
    {
        return &m_listen_ov;
    }

    inline SOCKET get_socket()
    {
        return m_socket;
    }

    inline SOCKET reset_socket()
    {
        return m_socket = INVALID_SOCKET;
    }

    inline std::vector<char>& get_buff()
    {
        return buff;
    }

    inline void set_callback(const std::function<void(std::shared_ptr<ddtcp_socket>)>& callback)
    {
        m_callback = callback;
    }

    inline void callback(const std::shared_ptr<ddtcp_socket>& socket)
    {
        if (m_callback != nullptr) {
            m_callback(socket);
        }
    }
private:
    SOCKET m_socket = INVALID_SOCKET;
    std::vector<char> buff;
    OVERLAPPED m_listen_ov{ };
    bool m_ipv4_6 = false;
    std::function<void(std::shared_ptr<ddtcp_socket>)> m_callback;
};

std::shared_ptr<ddtcp_acceptor> ddtcp_acceptor::create_inst(ddiocp_with_dispatcher* iocp, const ddaddr& listen_addr)
{
    if (iocp == nullptr) {
        return nullptr;
    }

    std::shared_ptr<ddtcp_acceptor> inst(new(std::nothrow)ddtcp_acceptor());
    if (inst == nullptr) {
        return nullptr;
    }

    inst->m_ipv4_6 = listen_addr.ipv4_6;
    if (!inst->init(listen_addr)) {
        return false;
    }

    if (!iocp->watch(inst)) {
        return false;
    }
    return inst;
}

bool ddtcp_acceptor::init(const ddaddr& listen_addr)
{
    m_socket = ddnetwork_utils::create_async_tcp_socket(listen_addr.ipv4_6);
    if (m_socket == INVALID_SOCKET) {
        return false;
    }

    // 获取AcceptEx函数指针
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes = 0;
    if (::WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
        &m_lpfn_accecpex, sizeof(m_lpfn_accecpex), &dwBytes, NULL, NULL) == SOCKET_ERROR) {
        return false;
    }

    // bind and listen
    if (!ddnetwork_utils::bind(m_socket, listen_addr)) {
        return false;
    }

    if (::listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
        return false;
    }

    return true;
}

ddtcp_acceptor::~ddtcp_acceptor()
{
    close_socket();
    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto it : m_pendings) {
        delete it.second;
    }
    m_pendings.clear();
}

void ddtcp_acceptor::close_socket()
{
    if (m_socket != INVALID_SOCKET) {
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

void ddtcp_acceptor::accept(const std::function<void(std::shared_ptr<ddtcp_socket>)>& callback, ddexpire expire /* = ddexpire::never */)
{
    DDASSERT(m_socket != INVALID_SOCKET);
    ddtcp_acceptor_item* item = nullptr;
    do {
        item = new(std::nothrow)ddtcp_acceptor_item();
        if (item == nullptr) {
            dderror_code::set_last_error(dderror_code::out_of_memory);
            break;
        }

        add_pending(item);

        if (!item->init()) {
            break;
        }

        set_expire(item->get_ov(), expire);
        item->set_callback(callback);
        DWORD len = 0;
        void* buff = item->get_buff().data();
        DWORD buff_size = (DWORD)item->get_buff().size();
        if (!((LPFN_ACCEPTEX)m_lpfn_accecpex)(m_socket, item->get_socket(), buff, 0, buff_size / 2, buff_size / 2, &len, item->get_ov()) &&
            ::WSAGetLastError() != ERROR_IO_PENDING) {
            set_expire(item->get_ov());
            break;
        }
        return;
    } while (0);

    if (callback != nullptr) {
        callback(nullptr);
    }

    if (item != nullptr) {
        remove_pending(item);
        delete item;
    }
}

ddcoroutine<std::shared_ptr<ddtcp_socket>> ddtcp_acceptor::accept(ddexpire expire /* = ddexpire::never */)
{
    std::shared_ptr<ddtcp_socket> return_socket;
    co_await ddawaitable([this, &return_socket, expire](const ddresume_helper& resumer) {
        accept([&return_socket, resumer](std::shared_ptr<ddtcp_socket> socket) {
            return_socket = socket;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_socket;
}

void ddtcp_acceptor::on_iocp_complete_v0(const ddiocp_item& item)
{
    ddtcp_acceptor_item* ov_item = get_item(item.overlapped);
    DDASSERT(ov_item != nullptr);
    remove_pending(ov_item);

    if (!item.has_error) {
        auto iocp = get_iocp();
        DDASSERT(iocp != nullptr);
        std::shared_ptr<ddsocket_friend> return_socket = std::make_shared<ddsocket_friend>();
        if (return_socket != nullptr) {
            return_socket->set_socket(ov_item->get_socket());
            ov_item->reset_socket();
            if (iocp->watch(return_socket)) {
                (void)::setsockopt(return_socket->get_socket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(SOCKET));
                ov_item->callback(return_socket);
            } else {
                ov_item->callback(nullptr);
            }
        } else {
            ov_item->callback(nullptr);
        }
    } else {
        ov_item->callback(nullptr);
    }

    delete ov_item;
}

void ddtcp_acceptor::on_timeout(OVERLAPPED* ov)
{
    ddtcp_acceptor_item* ov_item = get_item(ov);
    DDASSERT(ov_item != nullptr);
    dderror_code::set_last_error(dderror_code::time_out);
    remove_pending(ov_item);
    ov_item->callback(nullptr);
    delete ov_item;
}

void ddtcp_acceptor::add_pending(ddtcp_acceptor_item* item)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    DDASSERT(item != nullptr);
    m_pendings[item->get_ov()] = item;
}

void ddtcp_acceptor::remove_pending(ddtcp_acceptor_item* item)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    DDASSERT(item != nullptr);
    m_pendings.erase(item->get_ov());
}

ddtcp_acceptor_item* ddtcp_acceptor::get_item(OVERLAPPED* ov)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    DDASSERT(ov != nullptr);
    auto it = m_pendings.find(ov);
    if (it == m_pendings.end()) {
        return nullptr;
    }
    return it->second;
}
} // namespace NSP_DD

//////////////////////////////////ddtcp_connector//////////////////////////////////
namespace NSP_DD {
class ddsocket_connector_item : public ddtcp_socket
{
public:
    ddsocket_connector_item(const std::function<void(std::shared_ptr<ddtcp_socket>)>& callback, ddtcp_connector* connector) :
        m_callback(callback), m_connector(connector)
    {
    }

    ~ddsocket_connector_item()
    {
        if (m_ov.hEvent != NULL) {
            ::WSACloseEvent(m_ov.hEvent);
            m_ov.hEvent = NULL;
        }
    }

    void on_timeout(OVERLAPPED* ov) final
    {
        if (ov != &m_ov) {
            ddtcp_socket::on_timeout(ov);
            return;
        }

        dderror_code::set_last_error(dderror_code::time_out);
        m_connector->remove_pending(this);
        m_callback(nullptr);
    }

    void on_iocp_complete_v1(const ddiocp_item& item) final
    {
        m_connector->remove_pending(this);

        if (!item.has_error) {
            std::shared_ptr<ddsocket_friend> socket = std::make_shared<ddsocket_friend>();
            if (socket != nullptr) {
                socket->set_socket(m_socket);
                m_socket = INVALID_SOCKET;
                (void)transfer_to(socket);
                (void)::setsockopt(socket->get_socket(), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
                m_callback(socket);
            } else {
                m_callback(nullptr);
            }
        } else {
            m_callback(nullptr);
        }
    }

    HANDLE get_handle() override
    {
        return (HANDLE)m_socket;
    }

    bool connect(const ddaddr& addr, ddexpire expire)
    {
        m_ov.hEvent = ::WSACreateEvent();
        if (m_ov.hEvent == NULL) {
            return false;
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
            return false;
        }

        // get the ConnectEx function.
        DWORD dummy = 0;
        void* lpfn_connectex = nullptr;
        ::GUID guid = WSAID_CONNECTEX;
        (void)::WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid, sizeof(GUID),
            &lpfn_connectex, sizeof(lpfn_connectex),
            &dummy, NULL, NULL);
        if (lpfn_connectex == NULL) {
            return false;
        }

        // call teh ConnectEx function.
        set_expire(&m_ov, expire);
        if (!((LPFN_CONNECTEX(lpfn_connectex))(m_socket, connect_addr, addr_len, NULL, 0, &dummy, &m_ov)) &&
            ::WSAGetLastError() != ERROR_IO_PENDING) {
            set_expire(&m_ov);
            return false;
        }
        return true;
    }

    bool bind(const ddaddr& addr)
    {
        auto socket = ddnetwork_utils::create_async_tcp_socket(addr.ipv4_6);
        if (socket == INVALID_SOCKET) {
            return false;
        }

        m_socket = socket;

        // ConnectEx need the socked had been bound;
        // https://learn.microsoft.com/en-us/windows/win32/api/mswsock/nc-mswsock-lpfn_connectex#:~:text=A%20descriptor%20that%20identifies%20an%20unconnected%2C%20previously%20bound%20socket.%20See%20Remarks%20for%20more%20information.
        if (!ddnetwork_utils::bind(m_socket, ddaddr{ ddnetwork_utils::get_zero_ip(addr.ipv4_6), 0, addr.ipv4_6 })) {
            return false;
        }

        return true;
    }

protected:
    ::OVERLAPPED m_ov{};
    std::function<void(std::shared_ptr<ddtcp_socket>)> m_callback;
    ddtcp_connector* m_connector = nullptr;
};

std::shared_ptr<ddtcp_connector> ddtcp_connector::create_inst(ddiocp_with_dispatcher* iocp)
{
    if (iocp == nullptr) {
        return nullptr;
    }
    std::shared_ptr<ddtcp_connector> inst(new(std::nothrow)ddtcp_connector());
    if (inst == nullptr) {
        return nullptr;
    }

    inst->m_iocp = iocp;
    return inst;
}

ddtcp_connector::~ddtcp_connector()
{
    close_socket();
}

void ddtcp_connector::close_socket()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_pendings.clear();
}

void ddtcp_connector::add_pending(const std::shared_ptr<ddsocket_connector_item>& item)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    DDASSERT(item != nullptr);
    m_pendings.push_back(item);
}

void ddtcp_connector::remove_pending(ddsocket_connector_item* item)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    DDASSERT(item != nullptr);
    m_pendings.remove_if([item](const std::shared_ptr<ddsocket_connector_item>& it) {
        return item == it.get();
    });
}

void ddtcp_connector::connect_to(const ddaddr& addr, const std::function<void(const std::shared_ptr<ddtcp_socket>&)>& callback, ddexpire expire /* = ddexpire::never */)
{
    std::shared_ptr<ddsocket_connector_item> item;
    do {
        item = std::make_shared<ddsocket_connector_item>(callback, this);
        if (item == nullptr) {
            break;
        }

        add_pending(item);

        if (!item->bind(addr)) {
            break;
        }

        if (!m_iocp->watch(item)) {
            break;
        }

        if (!item->connect(addr, expire)) {
            break;
        }

        return;
    } while (0);

    if (item != nullptr) {
        remove_pending(item.get());
    }

    if (callback != nullptr) {
        callback(nullptr);
    }
}

ddcoroutine<std::shared_ptr<ddtcp_socket>> ddtcp_connector::connect_to(const ddaddr& addr, ddexpire expire /* = ddexpire::never */)
{
    std::shared_ptr<ddtcp_socket> return_socket = nullptr;
    co_await ddawaitable([this, &return_socket, addr, expire](const ddresume_helper& resumer) {
        connect_to(addr, [&return_socket, resumer](const std::shared_ptr<ddtcp_socket>& socket) {
            return_socket = socket;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_socket;
}
} // namespace NSP_DD
