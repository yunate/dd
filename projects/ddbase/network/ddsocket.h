#ifndef ddbase_network_ddsocket_h_
#define ddbase_network_ddsocket_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"

#include "ddbase/network/ddnetwork_utils.h"
#include <memory>

namespace NSP_DD {
class ddsocket : public ddiocp_io_dispatch
{
    ddsocket() = delete;
    friend class ddsocket_connector;
public:
    virtual ~ddsocket();

    inline HANDLE get_handle() override
    {
        return (HANDLE)m_socket;
    }

protected:
    SOCKET m_socket = INVALID_SOCKET;

    DDNO_COPY_MOVE(ddsocket);
};
} // namespace NSP_DD
#endif // ddbase_network_ddsocket_h_