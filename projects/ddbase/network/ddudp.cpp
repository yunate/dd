#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/ddudp.h"
#include "ddbase/ddexec_guard.hpp"

namespace NSP_DD {
std::shared_ptr<ddudp_socket> ddudp_socket::create_inst(ddiocp_with_dispatcher* iocp, bool ipv4_6 /* = true */)
{
    std::shared_ptr<ddudp_socket> inst(new(std::nothrow)ddudp_socket());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    inst->m_socket = ddnetwork_utils::create_async_udp_socket(ipv4_6);
    if (inst->m_socket == INVALID_SOCKET) {
        return nullptr;
    }

    if (!iocp->watch(inst)) {
        return nullptr;
    }

    inst->m_ipv4_6 = ipv4_6;
    return inst;
}

ddudp_socket::~ddudp_socket()
{
    close_socket();
}

void ddudp_socket::close_socket()
{
    if (m_socket != INVALID_SOCKET) {
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

bool ddudp_socket::listen(u16 port)
{
    ddaddr addr;
    addr.ip = ddnetwork_utils::get_zero_ip(m_ipv4_6);
    addr.port = port;
    ::sockaddr* paddr = nullptr;
    ::sockaddr_in connect_addr4{};
    ::sockaddr_in6 connect_addr6{};
    int addr_len = 0;
    if (m_ipv4_6) {
        paddr = (sockaddr*)&connect_addr4;
        addr_len = sizeof(sockaddr_in);
    } else {
        paddr = (sockaddr*)&connect_addr6;
        addr_len = sizeof(sockaddr_in6);
    }

    if (!addr.to_sockaddr(paddr)) {
        return false;
    }

    if (::bind(m_socket, paddr, addr_len) != 0) {
        return false;
    }

#ifdef _DEBUG
    m_has_listened = true;
#endif
    return true;
}

ddcoroutine<std::tuple<bool, s32>> ddudp_socket::send_to(void* buff, s32 buff_size, const ddaddr& addr, ddexpire expire /* = ddexpire::never */)
{
    auto pred = [this, buff, buff_size, addr](ddiiocp_dispatch* dispatch, OVERLAPPED* ov) {
        DDASSERT(dispatch != nullptr);
        auto handle = dispatch->get_handle();
        if (handle == NULL) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        ::sockaddr* p_addr = nullptr;
        int addr_len = 0;
        ::sockaddr_in addr4{};
        ::sockaddr_in6 addr6{};
        if (addr.ipv4_6) {
            p_addr = (sockaddr*)&addr4;
            addr_len = sizeof(sockaddr_in);
        }
        else {
            p_addr = (sockaddr*)&addr6;
            addr_len = sizeof(sockaddr_in6);
        }

        if (!addr.to_sockaddr(p_addr)) {
            return false;
        }

        m_send_wsa_buff.buf = (char*)buff;
        m_send_wsa_buff.len = (LONG)buff_size;
        if (::WSASendTo((SOCKET)handle, &m_send_wsa_buff, 1, NULL, 0, p_addr, addr_len, ov, NULL) != 0
            && ::GetLastError() != ERROR_IO_PENDING) {
            return false;
        }

        return true;
    };

    co_return co_await ddiocp_io_dispatch_base::write(pred, expire);
}

ddcoroutine<std::tuple<bool, s32>> ddudp_socket::recv_from(void* buff, s32 buff_size, ddaddr& addr, ddexpire expire /* = ddexpire::never */)
{
#ifdef _DEBUG
    DDASSERT_FMTW(m_has_listened, L"the listen() function should be called before call recv_from() function.");
#endif
    auto pred = [this, buff, buff_size](ddiiocp_dispatch* dispatch, OVERLAPPED* ov) {
        DDASSERT(dispatch != nullptr);
        auto handle = dispatch->get_handle();
        if (handle == NULL) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        recv_wsa_buff.buf = (char*)buff;
        recv_wsa_buff.len = (LONG)buff_size;
        m_flags = 0;
        if (::WSARecvFrom((SOCKET)handle, &recv_wsa_buff, 1, NULL, &m_flags, (::sockaddr*)m_addr_buff, &m_addr_buff_len, ov, NULL) != 0
            && ::GetLastError() != ERROR_IO_PENDING) {
            return false;
        }
        return true;
    };

    auto result =  co_await ddiocp_io_dispatch_base::read(pred, expire);
    if (std::get<0>(result)) {
        addr.from_sockaddr((::sockaddr*)m_addr_buff);
    }
    co_return result;
}
} // namespace NSP_DD
