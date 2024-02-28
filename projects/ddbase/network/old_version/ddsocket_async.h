#ifndef ddbase_network_ddsocket_async_h_
#define ddbase_network_ddsocket_async_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/stream/ddistream.h"

#include "ddbase/iocp/ddiocp.h"
#include "ddbase/network/ddsocket.h"

#include <list>
namespace NSP_DD {

using ddsocket_async_callback = std::function<void(bool successful, s32 sended)>;
using ddsocket_async_callback1 = std::function<bool(bool successful, bool all_sended, u64 sended)>;
class ddsocket_async : public ddsocket, public ddiiocp_dispatch
{
public:
    // @param callback: 一定会被调用, 除非iocp或者自己被释放了
    // @param[in] buff 由用户维护, 在callback回调前不能被释放
    // @note 如果上一次的send还没有完成, 那么这次的send会被--放到队列中--, 等上一次的send完成后, 再发送
    virtual void send(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout = INFINITE);

    // @param callback: 一定会被调用, 除非iocp或者自己被释放了
    // @note 如果上一次的send还没有完成, 那么这次的send会被--放到队列中--, 等上一次的send完成后, 再发送
    virtual void send_stream(ddistream* stream, const ddsocket_async_callback1& callback, u64 timeout = INFINITE);

    // @param callback: 一定会被调用, 除非iocp或者自己被释放了
    // @param[in] buff 由用户维护, 在callback回调前不能被释放
    // @note 如果上一次的recv还没有完成, 那么这次的recv会被--忽略--
    virtual void recv(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout = INFINITE);

    // 检查socket是否有效
    virtual bool test_socket();
public:
    // ddiiocp_dispatch
    virtual HANDLE get_handle() override;
    virtual void on_iocp_complete(const ddiocp_item& item) override;

private:
    struct pending_context
    {
        void* buff = nullptr;
        s32 buff_size = 0;
        u64 experid_timestamp = 0;
        ddsocket_async_callback callback;
    };

    // 调用该函数时, 由调用者设置m_recv_callback
    // 不管发送成功失败on_iocp_complete都将被调用
    void recv_inner(void* buff, s32 buff_size, u64 experid_timestamp);
    OVERLAPPED m_recv_ov{};
    ddsocket_async_callback m_recv_callback;

    // 调用该函数时, 由调用者设置m_send_callback
    // 不管发送成功失败on_iocp_complete都将被调用
    void send_inner(void* buff, s32 buff_size, u64 experid_timestamp);
    void do_pending_send();
    OVERLAPPED m_send_ov{};
    ddsocket_async_callback m_send_callback;
    std::list<pending_context*> m_pending_sends;

};

} // namespace NSP_DD
#endif // ddbase_network_ddsocket_async_h_
