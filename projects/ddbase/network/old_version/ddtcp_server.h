#ifndef ddbase_network_ddtcp_server_h_
#define ddbase_network_ddtcp_server_h_

#include "ddbase/network/ddtcp_accepter_async.h"

namespace NSP_DD {

class ddtcp_server
{
public:
    virtual ~ddtcp_server() {};
    static std::unique_ptr<ddtcp_server> create_instance(ddiocp_with_dispatcher& iocp, const std::function<void(std::shared_ptr<ddsocket_async>)>& client_connect_callback);

    virtual void listen_port(u16 port, bool ipv4_6, const std::function<void()>& on_error) = 0;
    virtual void unlisten_port(u16 port) = 0;
};

} // namespace NSP_DD
#endif // ddbase_network_ddtcp_server_h_
