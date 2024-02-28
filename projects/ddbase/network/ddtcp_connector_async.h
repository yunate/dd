#ifndef ddbase_network_ddtcp_connector_async_h_
#define ddbase_network_ddtcp_connector_async_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddnet_util.h"
#include "ddbase/network/ddsocket_async.h"
#include "ddbase/iocp/ddiocp.h"

namespace NSP_DD {
class ddtcp_connector_async_item : public ddsocket_async
{
public:
    virtual ~ddtcp_connector_async_item() {};
    static std::shared_ptr<ddtcp_connector_async_item> create_instance(bool ipv4_6, ddiocp_with_dispatcher& iocp);
    virtual bool connect(const ddaddr& addr, const std::function<void(bool)>& callback, u64 timeout) = 0;
};

class ddtcp_connector_async
{
public:
    virtual ~ddtcp_connector_async() {}
    static std::unique_ptr<ddtcp_connector_async> create_instance(bool ipv4_6, ddiocp_with_dispatcher& iocp);
    // @param[in] callback:
    //     如果参数ddsocket_async*返回nullptr, 表示失败.
    //         使用dderror_code::get_last_error()获得失败具体原因
    //         如果dderror_code::get_last_error() == dderror_code::time_out, 表示超时
    virtual bool connect(const ddaddr& addr, const std::function<void(std::shared_ptr<ddsocket_async>)>& callback, u64 timeout) = 0;
    bool connect(const ddaddr& addr, const std::function<void(std::shared_ptr<ddsocket_async>)>& callback)
    {
        return connect(addr, callback, INFINITE);
    }
};

} // namespace NSP_DD
#endif // ddbase_network_ddtcp_connector_async_h_

