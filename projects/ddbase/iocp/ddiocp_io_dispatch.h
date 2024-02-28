#ifndef ddbase_iocp_ddiocp_io_dispatch_h_
#define ddbase_iocp_ddiocp_io_dispatch_h_
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include "ddbase/thread/ddasync.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/stream/ddstream_view.h"

namespace NSP_DD {
class ddiocp_io_dispatch_base : public ddiiocp_dispatch
{
protected:
    virtual ~ddiocp_io_dispatch_base() = default;

    using ddio_pred = std::function<bool(ddiiocp_dispatch* dispatch, OVERLAPPED* ov)>;
    ddcoroutine<std::tuple<bool, s32>> write(const ddio_pred& pred, ddexpire expire);
    ddcoroutine<std::tuple<bool, s32>> read(const ddio_pred& pred, ddexpire expire);

protected:
    // form ddiiocp_dispatch
    void on_iocp_complete_v0(const ddiocp_item& item) final;
    virtual void on_iocp_complete_v1(const ddiocp_item& item) { item; };
    void on_timeout(OVERLAPPED* ov) override;

private:
    using ddio_callback = std::function<void(bool successful, s32 byte)>;
    struct ddio_impl {
        ddiiocp_dispatch* m_dispatch = nullptr;
        OVERLAPPED m_ov{};
        ddio_callback m_callback;
        void run(const ddio_pred& pred, const ddio_callback& callback, ddexpire expire);
        bool on_timeout(OVERLAPPED* ov);
        bool on_complete(const ddiocp_item& item);
    };

    ddio_impl m_read_impl { this };
    ddio_impl m_write_impl { this };
};
} // namespace NSP_DD

namespace NSP_DD {
class ddiocp_io_dispatch : public ddiocp_io_dispatch_base
{
public:
    virtual ~ddiocp_io_dispatch() = default;
    // @param[in] buff 由caller维护, 在函数返回前不能被释放
    // @note 如果上一次的write还没有完成, 那么这次的失败
    ddcoroutine<std::tuple<bool, s32>> write(void* buff, s32 buff_size, ddexpire expire = ddexpire::never);

    // @param[in] buff 由caller维护, 在函数返回前不能被释放
    // @note 如果上一次的read还没有完成, 那么这次的read的失败
    ddcoroutine<std::tuple<bool, s32>> read(void* buff, s32 buff_size, ddexpire expire = ddexpire::never);
};
} // namespace NSP_DD
#endif // ddbase_iocp_ddiocp_io_dispatch_h_
