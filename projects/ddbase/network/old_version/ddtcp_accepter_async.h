#ifndef ddbase_network_ddtcp_accepter_async_h_
#define ddbase_network_ddtcp_accepter_async_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddnet_util.h"
#include "ddbase/network/ddsocket_async.h"
#include "ddbase/iocp/ddiocp.h"

namespace NSP_DD {
class ddtcp_accepter_async
{
public:
    virtual ~ddtcp_accepter_async() {}
    static std::shared_ptr<ddtcp_accepter_async> create_instance(const ddaddr& addr, ddiocp_with_dispatcher& iocp);
    // @param[in] callback:
    //     如果参数ddsocket_async*返回nullptr, 表示失败.
    //         使用dderror_code::get_last_error()获得失败具体原因
    //         如果dderror_code::get_last_error() == dderror_code::time_out, 表示超时
    virtual bool accept_async(const std::function<void(std::shared_ptr<ddsocket_async>)>& callback, u64 timeout) = 0;
    bool accept_async(const std::function<void(std::shared_ptr<ddsocket_async>)>& callback)
    {
        return accept_async(callback, INFINITE);
    }
};

} // namespace NSP_DD

#endif // ddbase_network_ddtcp_accepter_async_h_
