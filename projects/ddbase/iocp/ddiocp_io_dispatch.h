#ifndef ddbase_iocp_ddiocp_io_dispatch_h_
#define ddbase_iocp_ddiocp_io_dispatch_h_
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include "ddbase/thread/ddasync.h"
#include "ddbase/stream/ddstream_view.h"

namespace NSP_DD {
class ddiocp_io_dispatch_base : public ddiiocp_dispatch
{
protected:
    struct ddio_opt {
        // 执行的操作, 不可以设置为nullptr, 比如write/read file等, dispatch 一定不为nullptr
        std::function<bool(ddiiocp_dispatch* dispatch, OVERLAPPED*)> exec;
        // 是否操作已经结束, 可以指定为nullptr, 为nullptr的行为和永远返回true一致
        std::function<bool()> is_end;
        // 回调函数, 可能回调多次(取决于is_end的返回值)直到全部完成或者出错
        std::function<void(bool successful, s32 byte, bool is_end)> callback;
        // 为nullptr的话, exec会在当前线程调用, 如果指定async_caller, 那么exec会在async_caller调用
        ddasync_caller async_caller = nullptr;
    };

    virtual ~ddiocp_io_dispatch_base() = default;
    void write(const ddio_opt& opt, ddexpire expire);
    void read(const ddio_opt& opt, ddexpire expire);

protected:
    // form ddiiocp_dispatch
    void on_iocp_complete_v0(const ddiocp_item& item) final;
    virtual void on_iocp_complete_v1(const ddiocp_item& item) { item; };
    void on_timeout(OVERLAPPED* ov) override;

private:
    struct ddio_impl
    {
        void run(ddiocp_with_dispatcher* iocp, const std::shared_ptr<ddiiocp_dispatch>& dispatch, const ddio_opt& opt, ddexpire expire);
        void continue_run(bool run_in_async_call);
        bool on_iocp_complete(const ddiocp_item& item);
        bool set_ctx(const ddio_opt& opt, ddiiocp_dispatch* dispatch, ddexpire expire);
        void reset_ctx();
        void on_opt(bool successful, s32 byte);
        bool on_timeout(OVERLAPPED* ov);

        OVERLAPPED m_ov{};
        ddio_opt m_opt;
        ddiiocp_dispatch* m_dispatch = nullptr;
        ddexpire m_expire;
    };

    ddio_impl m_write_impl;
    ddio_impl m_read_impl;
};
} // namespace NSP_DD

namespace NSP_DD {
class ddiocp_io_dispatch : public ddiocp_io_dispatch_base
{
public:
    virtual ~ddiocp_io_dispatch() = default;
    // @param[in] buff 由caller维护, 在callback回调前不能被释放
    // @note 如果上一次的write还没有完成, 那么这次的失败
    void write(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<std::tuple<bool, s32>> write(void* buff, s32 buff_size, ddexpire expire = ddexpire::never);

    // @param[in] stream_view 由caller维护, 在callback回调结束前不能被释放
    // @param[in] callback 可能会回调多次, 直到所有数据都写完或者出错
    // @param[in] async_caller 用于异步调用callback, 为nullptr的话, callback会在当前线程调用
    void write(ddistream_view* stream_view,
        const std::function<void(bool successful, s32 byte, bool all_writed)>& callback,
        ddasync_caller async_caller = nullptr, ddexpire expire = ddexpire::never);

    // @param[in] buff 由caller维护, 在callback回调前不能被释放
    // @note 如果上一次的read还没有完成, 那么这次的read的失败
    void read(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<std::tuple<bool, s32>> read(void* buff, s32 buff_size, ddexpire expire = ddexpire::never);
};
} // namespace NSP_DD
#endif // ddbase_iocp_ddiocp_io_dispatch_h_
