
#ifndef ddbase_network_ddtcp_connector_sync_h_
#define ddbase_network_ddtcp_connector_sync_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddnet_util.h"
#include "ddbase/network/ddsocket_sync.h"

#include <memory>

namespace NSP_DD {

class ddtcp_connector_sync
{
public:
    virtual ~ddtcp_connector_sync(){}
    static std::unique_ptr<ddtcp_connector_sync> create_instance(bool ipv4_6);
    virtual std::unique_ptr<ddsocket_sync> connect(const ddaddr& addr) = 0;
};

} // namespace NSP_DD
#endif // ddbase_network_ddtcp_connector_sync_h_
