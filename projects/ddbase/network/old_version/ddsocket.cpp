#include "ddbase/stdafx.h"
#include "ddbase/network/ddsocket.h"
#include "ddbase/dderror_code.h"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
ddsocket::~ddsocket()
{
    if (m_socket != INVALID_SOCKET) {
        dderror_code_guard guard;
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

SOCKET ddsocket::get_socket()
{
    return m_socket;
}

bool ddsocket::get_local_addr(ddaddr& addr)
{
    return ddnet_utils::get_socket_local(m_socket, addr);
}

bool ddsocket::get_remote_addr(ddaddr& addr)
{
    return ddnet_utils::get_socket_remote(m_socket, addr);
}
} // namespace NSP_DD