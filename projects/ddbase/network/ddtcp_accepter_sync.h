
#ifndef ddbase_network_ddtcp_accepter_sync_h_
#define ddbase_network_ddtcp_accepter_sync_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddnet_util.h"
#include "ddbase/network/ddsocket_sync.h"

namespace NSP_DD {
class ddtcp_accepter_sync
{
public:
    virtual ~ddtcp_accepter_sync() {}
    static std::unique_ptr<ddtcp_accepter_sync> create_instance(const ddaddr& addr);
    virtual std::unique_ptr<ddsocket_sync> accept() = 0;
};

} // namespace NSP_DD

#endif // ddbase_network_ddtcp_accepter_sync_h_
