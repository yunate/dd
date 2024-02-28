#include "ddbase/stdafx.h"
#include "ddbase/network/ddnet_util.h"
#include "ddbase/dderror_code.h"
#include <ws2tcpip.h>
#include <ip2string.h>

namespace NSP_DD {
//////////////////////ddnet_addr//////////////////////////////////
const std::string& ddnet_utils::ip_anys(bool ipv4_6)
{
    static const std::string ipv4_any = "0.0.0.0";
    static const std::string ipv6_any = "0000:0000:0000:0000:0000:0000:0000:0000";
    if (ipv4_6) {
        return ipv4_any;
    }
    return ipv6_any;
}

bool ddnet_utils::ddaddr_to_sockaddr(const ddaddr& from, ::sockaddr* to)
{
    if (to == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return false;
    }

    if (from.ipv4_6) {
        ::sockaddr_in* addr = (::sockaddr_in*)to;
        if (::inet_pton(AF_INET, from.ip.c_str(), &(addr->sin_addr)) != 1) {
            return false;
        }
        addr->sin_family = AF_INET;
        addr->sin_port = ::htons(from.port);
    } else {
        ::sockaddr_in6* addr = (::sockaddr_in6*)to;
        if (::inet_pton(AF_INET6, from.ip.c_str(), &(addr->sin6_addr)) != 1) {
            return false;
        }
        addr->sin6_family = AF_INET6;
        addr->sin6_port = ::htons(from.port);
    }
    return true;
}

bool ddnet_utils::sockaddr_to_ddaddr(const::sockaddr* from, ddaddr& to)
{
    if (from == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return false;
    }

    sockaddr_in6* addr6 = (::sockaddr_in6*)from;
    if (addr6->sin6_family == AF_INET6) {
        to.ipv4_6 = false;
    } else {
        to.ipv4_6 = true;
    }

    if (to.ipv4_6) {
        sockaddr_in* addr = (::sockaddr_in*)from;
        to.ip.resize(16);
        (void)::RtlIpv4AddressToStringA(&(addr->sin_addr), (PSTR)to.ip.data());
        to.ip = to.ip.c_str();
        to.port = ::ntohs(addr->sin_port);
    } else {
        sockaddr_in6* addr = (::sockaddr_in6*)from;
        to.ip.resize(46);
        (void)::RtlIpv6AddressToStringA(&(addr->sin6_addr), (PSTR)to.ip.data());
        to.ip = to.ip.c_str();
        to.port = ::ntohs(addr->sin6_port);
    }

    to.ip.shrink_to_fit();
    return !to.ip.empty();
}

bool ddnet_utils::get_socket_local(SOCKET socket, ddaddr& addr)
{
    if (socket == INVALID_SOCKET || socket == NULL) {
        dderror_code::set_last_error(dderror_code::param_invalid_handle);
        return false;
    }

    int dummy = sizeof(sockaddr_in);
    ::sockaddr_in addr4{};
    if (::getsockname(socket, (sockaddr*)&addr4, &dummy) == 0) {
        addr.ipv4_6 = true;
        return ddnet_utils::sockaddr_to_ddaddr((sockaddr*)&addr4, addr);
    }

    dummy = sizeof(sockaddr_in6);
    ::sockaddr_in6 addr6{};
    if (::getsockname(socket, (sockaddr*)&addr6, &dummy) == 0) {
        addr.ipv4_6 = false;
        return ddnet_utils::sockaddr_to_ddaddr((sockaddr*)&addr6, addr);
    }

    return false;
}

bool ddnet_utils::get_socket_remote(SOCKET socket, ddaddr& addr)
{
    if (socket == INVALID_SOCKET || socket == NULL) {
        dderror_code::set_last_error(dderror_code::param_invalid_handle);
        return false;
    }

    int dummy = sizeof(sockaddr_in);
    ::sockaddr_in addr4{};

    if (::getpeername(socket, (sockaddr*)&addr4, &dummy) == 0) {
        addr.ipv4_6 = true;
        return ddnet_utils::sockaddr_to_ddaddr((sockaddr*)&addr4, addr);
    }

    dummy = sizeof(sockaddr_in6);
    ::sockaddr_in6 addr6{};
    if (::getpeername(socket, (sockaddr*)&addr6, &dummy) == 0) {
        addr.ipv4_6 = false;
        return ddnet_utils::sockaddr_to_ddaddr((sockaddr*)&addr6, addr);
    }

    return false;
}

bool ddnet_utils::bind(SOCKET socket, const ddaddr& addr)
{
    if (socket == INVALID_SOCKET || socket == NULL) {
        dderror_code::set_last_error(dderror_code::param_invalid_handle);
        return false;
    }

    ::sockaddr* p_addr = nullptr;
    ::sockaddr_in addr4{};
    ::sockaddr_in6 addr6{};
    int addr_len = 0;
    if (addr.ipv4_6) {
        p_addr = (sockaddr*)&addr4;
        addr_len = sizeof(sockaddr_in);
    } else {
        p_addr = (sockaddr*)&addr6;
        addr_len = sizeof(sockaddr_in6);
    }

    if (!ddnet_utils::ddaddr_to_sockaddr(addr, p_addr)) {
        return false;
    }

    if (::bind(socket, p_addr, addr_len) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

} // namespace NSP_DD
