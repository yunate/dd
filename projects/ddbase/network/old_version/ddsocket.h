#ifndef ddbase_network_ddsocket_h_
#define ddbase_network_ddsocket_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/network/ddnet_util.h"

namespace NSP_DD {
class ddsocket
{
    DDNO_COPY_MOVE(ddsocket);
public:
    ddsocket() = default;
    virtual ~ddsocket();

    SOCKET get_socket();
    bool get_local_addr(ddaddr& addr);
    bool get_remote_addr(ddaddr& addr);

protected:
    SOCKET m_socket = INVALID_SOCKET;
};
} // namespace NSP_DD
#endif // ddbase_network_ddsocket_h_
