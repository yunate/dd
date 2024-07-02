#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/ddsocket.h"

#include <mswsock.h>

namespace NSP_DD {
ddsocket::~ddsocket()
{
    if (m_socket != INVALID_SOCKET) {
        auto* iocp = get_iocp();
        if (iocp != nullptr) {
            iocp->unwatch(this);
        }

        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}
} // namespace NSP_DD