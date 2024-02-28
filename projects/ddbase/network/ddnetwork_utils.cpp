#include "ddbase/stdafx.h"
#include "ddbase/network/ddnetwork_utils.h"
#include "ddbase/dderror_code.h"
#include <ws2tcpip.h>
#include <ip2string.h>

namespace NSP_DD {
ddnetwork_initor::~ddnetwork_initor()
{
    if (m_WSAStartup_result == 0) {
        ::WSACleanup();
    }
}

bool ddnetwork_initor::init(u8 v0, u8 v1)
{
    if (!init_wsa(v0, v1)) {
        return false;
    }

    return true;
}

bool ddnetwork_initor::init_wsa(u8 v0, u8 v1)
{
    WSADATA wsaData;
    m_WSAStartup_result = ::WSAStartup(MAKEWORD(v0, v1), &wsaData);
    if (m_WSAStartup_result != 0) {
        return false;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        ::WSACleanup();
        return false;
    }

    return true;
}
} // namespace NSP_DD

namespace NSP_DD {
    bool ddaddr::to_sockaddr(::sockaddr* to) const
    {
        if (to == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            return false;
        }

        if (ipv4_6) {
            ::sockaddr_in* addr = (::sockaddr_in*)to;
            if (::inet_pton(AF_INET, ip.c_str(), &(addr->sin_addr)) != 1) {
                return false;
            }
            addr->sin_family = AF_INET;
            addr->sin_port = ::htons(port);
        } else {
            ::sockaddr_in6* addr = (::sockaddr_in6*)to;
            if (::inet_pton(AF_INET6, ip.c_str(), &(addr->sin6_addr)) != 1) {
                return false;
            }
            addr->sin6_family = AF_INET6;
            addr->sin6_port = ::htons(port);
        }
        return true;
    }

    bool ddaddr::from_sockaddr(const ::sockaddr* from)
    {
        if (from == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            return false;
        }

        sockaddr_in6* addr6 = (::sockaddr_in6*)from;
        if (addr6->sin6_family == AF_INET6) {
            ipv4_6 = false;
        } else {
            ipv4_6 = true;
        }

        if (ipv4_6) {
            sockaddr_in* addr = (::sockaddr_in*)from;
            ip.resize(16);
            (void)::RtlIpv4AddressToStringA(&(addr->sin_addr), (PSTR)ip.data());
            port = ::ntohs(addr->sin_port);
        } else {
            sockaddr_in6* addr = (::sockaddr_in6*)from;
            ip.resize(46);
            (void)::RtlIpv6AddressToStringA(&(addr->sin6_addr), (PSTR)ip.data());
            port = ::ntohs(addr->sin6_port);
        }

        ip = ip.c_str();
        ip.shrink_to_fit();
        return !ip.empty();
    }
}

namespace NSP_DD {
const std::string& ddnetwork_utils::get_zero_ip(bool ipv4_6)
{
    static const std::string ipv4_any = "0.0.0.0";
    static const std::string ipv6_any = "0000:0000:0000:0000:0000:0000:0000:0000";
    if (ipv4_6) {
        return ipv4_any;
    }
    return ipv6_any;
}

bool ddnetwork_utils::get_local_addr(SOCKET socket, ddaddr& addr)
{
    if (socket == INVALID_SOCKET || socket == NULL) {
        dderror_code::set_last_error(dderror_code::param_invalid_handle);
        return false;
    }

    int dummy = sizeof(sockaddr_in);
    ::sockaddr_in addr4{};
    if (::getsockname(socket, (sockaddr*)&addr4, &dummy) == 0) {
        return addr.from_sockaddr((sockaddr*)&addr4);
    }

    dummy = sizeof(sockaddr_in6);
    ::sockaddr_in6 addr6{};
    if (::getsockname(socket, (sockaddr*)&addr6, &dummy) == 0) {
        addr.ipv4_6 = false;
        return addr.from_sockaddr((sockaddr*)&addr6);
    }

    return false;
}

bool ddnetwork_utils::get_remote_addr(SOCKET socket, ddaddr& addr)
{
    if (socket == INVALID_SOCKET || socket == NULL) {
        dderror_code::set_last_error(dderror_code::param_invalid_handle);
        return false;
    }

    int dummy = sizeof(sockaddr_in);
    ::sockaddr_in addr4{};
    if (::getpeername(socket, (sockaddr*)&addr4, &dummy) == 0) {
        addr.ipv4_6 = true;
        return addr.from_sockaddr((sockaddr*)&addr4);
    }

    dummy = sizeof(sockaddr_in6);
    ::sockaddr_in6 addr6{};
    if (::getpeername(socket, (sockaddr*)&addr6, &dummy) == 0) {
        addr.ipv4_6 = false;
        return addr.from_sockaddr((sockaddr*)&addr6);
    }

    return false;
}

bool ddnetwork_utils::bind(SOCKET socket, const ddaddr& addr)
{
    if (socket == INVALID_SOCKET || socket == NULL) {
        dderror_code::set_last_error(dderror_code::param_invalid_handle);
        return false;
    }

    ::sockaddr* p_addr = nullptr;
    int addr_len = 0;
    ::sockaddr_in addr4{};
    ::sockaddr_in6 addr6{};
    if (addr.ipv4_6) {
        p_addr = (sockaddr*)&addr4;
        addr_len = sizeof(sockaddr_in);
    } else {
        p_addr = (sockaddr*)&addr6;
        addr_len = sizeof(sockaddr_in6);
    }

    if (!addr.to_sockaddr(p_addr)) {
        return false;
    }

    if (::bind(socket, p_addr, addr_len) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

SOCKET ddnetwork_utils::create_async_tcp_socket(bool ipv4_6)
{
    if (ipv4_6) {
        return ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    } else {
        return ::WSASocketW(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    }
}

SOCKET ddnetwork_utils::create_async_udp_socket(bool ipv4_6)
{
    if (ipv4_6) {
        return ::WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
    } else {
        return ::WSASocketW(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
    }
}
} // namespace NSP_DD