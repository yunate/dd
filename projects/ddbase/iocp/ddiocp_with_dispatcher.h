#ifndef ddbase_iocp_ddiocp_with_dispatcher_h_
#define ddbase_iocp_ddiocp_with_dispatcher_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/iocp/ddiocp.h"

#include <mutex>
#include <functional>
#include <windows.h>

namespace NSP_DD {
class ddiocp_with_dispatcher;
class ddiiocp_dispatch
{
public:
    virtual ~ddiiocp_dispatch()
    {
        // we do not call ddiocp_timeout_dispatch::unwatch() here, because its inherited class's object may have released when run to here.
        // so we relay on the caller to call ddiocp_timeout_dispatch::unwatch() function.
        DDASSERT_FMTW(get_iocp() == nullptr, L"ddiocp_timeout_dispatch::unwatch() function should be called before release this.");
    }

    // @return 能够绑定到iocp的句柄
    virtual HANDLE get_handle() = 0;

    // 操作完成了
    // @note it is run in the iocp thread.
    virtual void on_iocp_complete_v0(const ddiocp_item& item) = 0;

    // 处理idle, 子类可以覆写这个函数来实现超时
    // @note it is run in the iocp thread.
    virtual void on_idle(u64 epoch) { epoch; };

    // the iocp will be setted at ddiocp_timeout_dispatch::watch()/unwatch() function called,
    // and ddiocp_timeout_dispatch::watch()/unwatch() function is sync.
    inline ddiocp_with_dispatcher* get_iocp()
    {
        return m_iocp;
    }

    inline bool has_attached_to_iocp()
    {
        return m_iocp != nullptr;
    }
private:
    void set_iocp(ddiocp_with_dispatcher* iocp)
    {
        m_iocp = iocp;
    }
    friend ddiocp_with_dispatcher;
    ddiocp_with_dispatcher* m_iocp = nullptr;
};

class ddiocp_timeout_dispatch : public ddiiocp_dispatch
{
public:
    void on_iocp_complete_v0(const ddiocp_item& item) final;
    virtual void on_iocp_complete_v1(const ddiocp_item& item) { item; };

    // this OVERLAPPED will timeout, it should not be used before on_timeout is called.
    // @note it is run in the iocp thread.
    virtual void will_timeout(OVERLAPPED*) { }
    // this OVERLAPPED had really timeouted, and now, the OVERLAPPED can be re-used.
    // @note it is run in the iocp thread.
    virtual void on_timeout(OVERLAPPED*) { }

    void set_timeout(OVERLAPPED* ov, u64 timeout = INFINITE);

    // @note it is run in the iocp thread.
    void on_idle(u64 epoch) final;

private:
    struct ddiocp_ov_timeout
    {
        OVERLAPPED* ov = NULL;
        u64 expire_epoch = MAX_U64;
    };

    // the count of timeout ovs is very small, so it is not need to use set.
    std::list<ddiocp_ov_timeout> m_timeouts;
    std::list<OVERLAPPED*> m_pending_timeout_ovs;
};

class ddiocp_io_dispatch : public ddiocp_timeout_dispatch
{
public:
    ~ddiocp_io_dispatch();
    void on_iocp_complete_v1(const ddiocp_item& item) final;
    virtual void on_iocp_complete_v2(const ddiocp_item& item) { item; };

    // @param[in] buff 由caller维护, 在callback回调前不能被释放
    // @note 如果上一次的write还没有完成, 那么这次的失败
    void write(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, u64 timeout = INFINITE);

    // @param[in] buff 由caller维护, 在callback回调前不能被释放
    // @note 如果上一次的read还没有完成, 那么这次的read的失败
    void read(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, u64 timeout = INFINITE);

public:
    void on_timeout(OVERLAPPED* ov) override;

private:
    struct ddio_impl
    {
        void run_inner(ddiocp_io_dispatch* dispatch, void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, u64 experid_epoch, bool is_write);
        void run(ddiocp_io_dispatch* dispatch, void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, u64 timeout, bool is_write);
        bool on_iocp_complete(const ddiocp_item& item);
        void callback(bool successful, s32 byte);
        bool on_timeout(OVERLAPPED* ov);

        OVERLAPPED m_ov{};
        std::function<void(bool successful, s32 byte)> m_callback;
    };

    ddio_impl m_write_impl;
    ddio_impl m_read_impl;
};

class ddiocp_with_dispatcher
{
    ddiocp_with_dispatcher() = default;
public:
    ~ddiocp_with_dispatcher() = default;
    static std::unique_ptr<ddiocp_with_dispatcher> create_instance();

    // 关闭 iocp, 如果处于wait状态中, 会导致dispatch返回false
    bool notify_close();

    // @return 是否成功
    // @note 如果已经watch了, 返回true
    // @note dispatch 在析构前必须要调用unwatch来保证不会再有on_iocp_complete回调, 否则会导致访问已经delete的dispatch
    bool watch(ddiiocp_dispatch* dispatch);

    // @return 是否成功
    // @note 如果不存在, 返回true
    bool unwatch(ddiiocp_dispatch* dispatch);

    // 返回false表示iocp被关闭了, 此时应该停止pump退出循环
    bool dispatch();

    // 将任务投递到iocp线程中并唤醒wait去执行任务, 如果没有在waitting状态中,则会在之后的某次唤醒时执行
    void push_task(const std::function<void()>& task);

    // 获得idle间隔
    u64 get_idle_interval();
    // interval: [1000, 5000]毫秒
    void set_idle_interval(u64 interval = 5000);

    // @return 是否运行在iocp线程中
    bool is_run_in_iocp_thread() const;
private:
    bool init();
    void run_task();
    void idle();
    std::recursive_mutex m_mutex;
    std::unique_ptr<ddiocp> m_iocp;
    std::map<HANDLE, ddiiocp_dispatch*> m_dispatchs;
    std::list<std::function<void()>> m_tasks;
    u64 m_idle_interval = 0;
    u64 m_pre_idle_epoch = 0;
};

} // namespace NSP_DD
#endif // ddbase_iocp_ddiocp_with_dispatcher_h_
