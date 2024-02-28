#include "ddbase/stdafx.h"
#include "ddbase/network/ddsocket_sync.h"
#include "ddbase/dderror_code.h"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
u64 ddsocket_sync::send(void* buff, u64 buff_size)
{
    if (buff == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return 0;
    }

    if (m_socket == INVALID_SOCKET) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return 0;
    }

    DWORD writed = ::send(m_socket, (const char*)buff, (DWORD)buff_size, 0);
    if (writed == SOCKET_ERROR) {
        return 0;
    }

    return writed;
}

u64 ddsocket_sync::recv(void* buff, u64 buff_size)
{
    if (buff == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return 0;
    }

    if (m_socket == INVALID_SOCKET) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return 0;
    }

    DWORD readed = ::recv(m_socket, (char*)buff, (DWORD)buff_size, 0);
    if (readed == SOCKET_ERROR) {
        return 0;
    }
    return readed;
}

} // namespace NSP_DD