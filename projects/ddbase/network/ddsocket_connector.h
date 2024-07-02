#ifndef ddbase_network_ddsocket_connector_h_
#define ddbase_network_ddsocket_connector_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddsocket.h"

namespace NSP_DD {

class ddsocket_connector_item;
class ddsocket_connector
{
public:
    ~ddsocket_connector();
    // @param[in] callback:
    // socket 为nullptr时候标识失败
    // 使用dderror_code::get_last_error()获得失败具体原因
    // 如果dderror_code::get_last_error() == dderror_code::time_out, 表示超时
    void connect_to(const ddaddr& addr, ddiocp_with_dispatcher* iocp, const std::function<void(std::unique_ptr<ddsocket> socket)>& callback, u64 timeout);
    void add_pending(ddsocket_connector_item* item);
    void remove_pending(ddsocket_connector_item* item);
    static void set_socket(ddsocket& dst, SOCKET socket);

private:
    std::mutex m_mutex;
    std::list<ddsocket_connector_item*> m_pendings;
};

} // namespace NSP_DD
#endif // ddbase_network_ddsocket_connector_h_