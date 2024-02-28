#ifndef ddbase_network_ddtls_socket_async_h_
#define ddbase_network_ddtls_socket_async_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddsocket_async.h"
#include "ddbase/network/ddtls.h"

#include <memory>

namespace NSP_DD {

class ddtls_socket_async : public ddsocket_async
{
    ddtls_socket_async(const std::shared_ptr<ddtls>& tls, const std::shared_ptr<ddsocket_async>& socket);
public:
    static std::shared_ptr<ddtls_socket_async> create_instance(const std::shared_ptr<ddtls>& tls, const std::shared_ptr<ddsocket_async>& socket);
    virtual ~ddtls_socket_async();
    // @param callback: 一定会被调用, 除非iocp或者自己被释放了
    // @param[in] buff 由用户维护, 在callback回调前不能被释放
    // @note 如果上一次的send还没有完成, 那么这次的send会被放到队列中, 等上一次的send完成后, 再发送
    virtual void send(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout = INFINITE) override;

    // @param callback: 一定会被调用, 除非iocp或者自己被释放了
    // @note 如果上一次的send还没有完成, 那么这次的send会被放到队列中, 等上一次的send完成后, 再发送
    virtual void send_stream(ddistream* stream, const ddsocket_async_callback1& callback, u64 timeout = INFINITE) override;

    // @param callback: 一定会被调用, 除非iocp或者自己被释放了
    // @param[in] buff 由用户维护, 在callback回调前不能被释放
    // @note 如果上一次的recv还没有完成, 那么这次的recv会被放到队列中, 等上一次的send完成后, 再接受
    virtual void recv(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout = INFINITE) override;

    // 握手
    void hand_shake(const std::function<void(bool)>& callback);

    // from ddiiocp_dispatch
    virtual bool virtual_dispatch() override;

private:
    void continue_hand_shake_inner();
    void end_hand_shake(bool successful);

    std::shared_ptr<ddtls> m_tls;
    std::shared_ptr<ddsocket_async> m_socket;

    // m_buff 中数据,其中明文长度用m_decoded_size表示,密文长度用m_encoded_size表示
    // --------------------------------------------------------------
    // |00|*************************明文******|00|*****密文****|00000|
    // --------------------------------------------------------------
    s32 m_decoded_size = 0;
    s32 m_decoded_data_begin_pos = 0;
    s32 m_encoded_size = 0;
    s32 m_encoded_data_begin_pos = 0;
    std::vector<u8> m_buff;

    // 
    void* m_handshake_ctx = nullptr;
};

} // namespace NSP_DD

#endif // ddbase_network_ddtls_socket_async_h_
