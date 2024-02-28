#ifndef ddbase_iocp_ddiocp_with_dispatcher_h_
#define ddbase_iocp_ddiocp_with_dispatcher_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/iocp/ddiocp.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/ddtime.h"
#include "ddbase/thread/ddasync.h"

#include <mutex>
#include <tuple>
#include <functional>
#include <windows.h>

namespace NSP_DD {
class ddiocp_with_dispatcher;
class ddiiocp_dispatch
{
public:
    // @return 能够绑定到iocp的句柄
    virtual HANDLE get_handle() = 0;

    virtual ~ddiiocp_dispatch();

    void set_expire(OVERLAPPED* ov, ddexpire expire = ddexpire::never);

    // the iocp is seted when the call of ddiocp_timeout_dispatch::watch() return successful
    // and keep valid until release.
    inline ddiocp_with_dispatcher* get_iocp()
    {
        // 这个值在没有watch的时候是nullptr
        // 或者iocp析构的时候, dispatch也会被置为nullptr, 这个时候线程是不安全的, 所以要保证iocp的生命周期大于dispatch
        // 但是有一种情况下dispatch的生命周期大于iocp: 协程未被唤醒, 在程序退出时候会清理这些未被唤醒的协程的上下文, 这种情况下是安全的
        return m_iocp;
    }

protected:
    // 操作完成了
    // @note it is run in the iocp thread.
    virtual void on_iocp_complete_v0(const ddiocp_item& item) = 0;

    // 超时了
    virtual void on_timeout(OVERLAPPED*) { }

    // 处理idle, 子类可以覆写这个函数来实现用户侧超时
    virtual void on_idle(u64 epoch) { epoch; };

    // 将handle的所有移到另外一个dispatch中
    // @note get_handle() 的值需要有caller负责转移所有权
    void transfer_to(const std::weak_ptr<ddiiocp_dispatch>& r);

private:
    void on_watched(ddiocp_with_dispatcher* iocp, HANDLE key)
    {
        m_iocp = iocp;
        m_key = key;
    }
    friend ddiocp_with_dispatcher;
    ddiocp_with_dispatcher* m_iocp = nullptr;
    // 由于在析构的时候调用虚函数get_handle是UB, 所以这里记录一下watch时候的handle.
    HANDLE m_key = NULL;
};

class ddiocp_with_dispatcher
{
    ddiocp_with_dispatcher() = default;
public:
    ~ddiocp_with_dispatcher();
    static std::unique_ptr<ddiocp_with_dispatcher> create_instance(const ddasync_caller& async_caller = nullptr);

    // 返回false表示iocp被关闭了, 此时应该停止pump退出循环
    // 当idle超时或者iocp完成时返回true
    bool dispatch();

    // @return 是否成功
    // @note 如果已经watch了, 返回true
    bool watch(const std::weak_ptr<ddiiocp_dispatch>& dispatch);

    // @return 不存在, 则返回nullptr的weaker
    std::weak_ptr<ddiiocp_dispatch> get_weaker(ddiiocp_dispatch* dispatch);

    // 关闭 iocp, 如果处于wait状态中, 会导致dispatch返回false
    bool notify_close();

    // 将任务投递到iocp线程中并唤醒wait去执行任务, 如果没有在waitting状态中,则会在之后的某次唤醒时执行
    void push_task(const std::function<void()>& task);

    // 获得idle间隔
    u64 get_idle_interval();

    // interval: [1000, 5000]毫秒
    void set_idle_interval(u64 interval = 5000);

    // @return 是否运行在iocp线程中
    bool is_run_in_iocp_thread() const;

private:
    // 如果希望见将handle的所有权由一个已经watched的dispatch中转移到另外一个dispatch中
    // 可以使用virtual_watch, 因为dispatch中的handle已经和iocp绑定了
    bool virtual_watch(const std::weak_ptr<ddiiocp_dispatch>& dispatch);

    // 只会被ddiiocp_dispatch的析构函数调用
    // the handle remains associated with that I/O completion port until it is closed.
    // https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-createiocompletionport
    void unwatch(ddiiocp_dispatch* dispatch);
    bool init();

    std::recursive_mutex m_mutex;
    std::unique_ptr<ddiocp> m_iocp;
    std::map<HANDLE, std::weak_ptr<ddiiocp_dispatch>> m_dispatchs;

    /// task
    void run_task();
    std::list<std::function<void()>> m_tasks;

    /// idle
    void idle();
    u64 m_idle_interval = 0;
    u64 m_pre_idle_epoch = 0;

    /// timeout
    void handle_timeout_on_idle(u64 now);
    void set_expire(OVERLAPPED* ov, HANDLE handle, ddexpire expire = ddexpire::never);
    //inline void set_timeout(OVERLAPPED* ov, HANDLE handle, u32 timeout = INFINITE)
    //{
    //    set_expire(ov, handle, ddexpire::form_timeout(timeout));
    //}
    void remove_timeout_on_handle(HANDLE handle);
    struct ddiocp_ov_timeout
    {
        ddexpire expire = ddexpire::never;
        OVERLAPPED* ov = NULL; // 唯一
        HANDLE handle = NULL; // 一个handle可以会有多个ov操作
    };

    std::list<ddiocp_ov_timeout> m_expire_epochs;
    std::list<ddiocp_ov_timeout> m_pending_timeout_ovs;

    ddasync_caller m_async_caller = nullptr;
    friend ddiiocp_dispatch;
};

} // namespace NSP_DD
#endif // ddbase_iocp_ddiocp_with_dispatcher_h_
