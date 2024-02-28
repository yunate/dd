#ifndef ddbase_network_ddsocket_sync_h_
#define ddbase_network_ddsocket_sync_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"

#include "ddbase/network/ddsocket.h"

namespace NSP_DD {
class ddsocket_sync : public ddsocket
{
    DDNO_COPY_MOVE(ddsocket_sync);
public:
    ddsocket_sync() = default;
    virtual ~ddsocket_sync() = default;

    // @param[in] buff 由用户维护
    // @param[in] buff_size buff的大小
    // @return 返回写入的字符数, 为0时表示失败, 比如远程socket关闭
    virtual u64 send(void* buff, u64 buff_size);

    // @param[in] buff 由用户维护
    // @param[in] buff_size buff的大小
    // @return 返回读出的字符数, 为0时表示失败, 比如远程socket关闭
    virtual u64 recv(void* buff, u64 buff_size);
};
} // namespace NSP_DD
#endif // ddbase_network_ddsocket_sync_h_
