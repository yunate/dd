#ifndef ddbase_network_ddnetwork_util_h_
#define ddbase_network_ddnetwork_util_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"

// 由于下面的头文件中包含了winsock2.h, 会和winsock.h冲突, 所以需要在包含该文件之前包含winsock.h或者定义宏 #define _WINSOCKAPI_
#include <winsock.h>
#include <ws2tcpip.h>
#include <ip2string.h>
#include <windows.h>

#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"Ntdll.lib")

namespace NSP_DD {
struct ddaddr
{
    std::string ip;
    u16 port = 0;
    bool ipv4_6 = true; // ipv4: true, ipv6: false

    // @note ::sockaddr, 可以由::in_addr/::in6_addr强转而来
    bool to_sockaddr(::sockaddr* to) const;
    bool from_sockaddr(const ::sockaddr* from);
};

class ddnetwork_utils
{
public:
    // INADDR_ANY
    static const std::string& get_zero_ip(bool ipv4_6);

    // 获取socket的本地/远程地址
    // @param[in] socket, 调用者保证有效
    static bool get_local_addr(SOCKET socket, ddaddr& addr);
    static bool get_remote_addr(SOCKET socket, ddaddr& addr);

    // 绑定socket到给定的ip port
    static bool bind(SOCKET socket, const ddaddr& addr);
};
} // namespace NSP_DD

#endif // ddbase_network_ddnetwork_util_h_
