#ifndef ddbase_network_ddtls_socket_sync_h_
#define ddbase_network_ddtls_socket_sync_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddtls.h"
#include "ddbase/network/ddsocket_sync.h"

#include <memory>

namespace NSP_DD {

class ddtls_socket_sync : public ddsocket_sync
{
public:
    ddtls_socket_sync(const std::shared_ptr<ddtls>& tls, const std::shared_ptr<ddsocket_sync>& socket);
    // @param[in] buff 由用户维护
    // @param[in] buff_size buff的大小
    // @return 返回写入的字符数, 为0时表示失败, 比如远程socket关闭
    virtual u64 send(void* buff, u64 buff_size);

    // @param[in] buff 由用户维护
    // @param[in] buff_size buff的大小
    // @return 返回读出的字符数, 为0时表示失败, 比如远程socket关闭
    virtual u64 recv(void* buff, u64 buff_size);

    // 握手
    bool hand_shake();

private:
    std::shared_ptr<ddtls> m_tls;
    std::shared_ptr<ddsocket_sync> m_socket;

    // m_buff 中数据,其中明文长度用m_decoded_size表示,密文长度用m_encoded_size表示
    // --------------------------------------------------------------
    // |00|*************************明文******|00|*****密文****|00000|
    // --------------------------------------------------------------
    s32 m_decoded_size = 0;
    s32 m_decoded_data_begin_pos = 0;
    s32 m_encoded_size = 0;
    s32 m_encoded_data_begin_pos = 0;
    std::vector<u8> m_buff;
};

} // namespace NSP_DD

#endif // ddbase_network_ddtls_socket_sync_h_
