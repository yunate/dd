
#include "ddbase/stdafx.h"
#include "ddbase/network/ddnetwork_initor.h"
#include "ddbase/network/ddnetwork_async_caller.hpp"


#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {

ddnetwork_initor::~ddnetwork_initor()
{
    if (m_WSAStartup_result == 0) {
        ::WSACleanup();
    }
}

bool ddnetwork_initor::init(const ddasync_caller& async_caller, u8 v0, u8 v1)
{
    if (!init_wsa(v0, v1)) {
        return false;
    }

    ddnetwork_async_caller::set_caller(async_caller);
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

