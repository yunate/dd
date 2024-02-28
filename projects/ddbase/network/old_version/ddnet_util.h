#ifndef ddbase_network_ddnet_util_h_
#define ddbase_network_ddnet_util_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"

#include <windows.h>

#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib,"Ntdll.lib")

// 调用该文件需要包含以下头文件, 由于下面的头文件中包含了winsock2.h, 会和winsock.h冲突, 所以需要在包含该文件之前包含winsock.h
// 或者定义宏 #define _WINSOCKAPI_
// #include <ws2tcpip.h>
// #include <ip2string.h>

struct in_addr;
struct in6_addr;
struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;
typedef UINT_PTR SOCKET;
#define INVALID_SOCKET  (SOCKET)(~0)

namespace NSP_DD {

//////////////////////ddnet_addr//////////////////////////////////
struct ddaddr
{
    std::string ip;
    u16 port = 0;

    // ipv4: true, ipv6: false
    bool ipv4_6 = true;
};

class ddnet_utils
{
public:
    // INADDR_ANY
    static const std::string& ip_anys(bool ipv4_6);

    // ddaddr和::sockaddr相互转换
    // @note ::sockaddr, 可以由::in_addr/::in6_addr强转而来
    static bool ddaddr_to_sockaddr(const ddaddr& from, ::sockaddr* to);
    static bool sockaddr_to_ddaddr(const ::sockaddr* from, ddaddr& to);

    // 获取socket的本地/远程地址
    // @param[out] socket, 调用者保证有效
    static bool get_socket_local(SOCKET socket, ddaddr& addr);
    static bool get_socket_remote(SOCKET socket, ddaddr& addr);

    // 绑定socket到给定的ip port
    static bool bind(SOCKET socket, const ddaddr& addr);
};
} // namespace NSP_DD

#endif // ddbase_network_ddnet_util_h_

