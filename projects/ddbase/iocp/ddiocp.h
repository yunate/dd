#ifndef ddbase_iocp_ddiocp_h_
#define ddbase_iocp_ddiocp_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include <mutex>
#include <functional>
#include <windows.h>

namespace NSP_DD {
class ddiocp;

////////////////////////////////ddiocp//////////////////////////////////////////
struct ddiocp_item
{
    // 如果为true, 说明某个端口完成了, 但是有错误, 比如连接断开等
    bool has_error = false;
    DWORD error_code = 0;
    std::size_t key = 0;
    u32 transferred_number = 0;
    OVERLAPPED* overlapped = NULL;
};

enum class ddiocp_notify_type
{
    // 有端口完成了
    complete = 0,

    // iocp被关闭了, 此时应该停止pump退出循环
    closed,

    // 超时
    timeout,

    // 内部任务导致端口完成, 调用者可以忽略该类型继续pump
    inner_task_run
};

class ddiocp
{
public:
    virtual ~ddiocp() = default;

    // 创建iocp
    // @param thread_count: 线程数, 0表示使用cpu核心数
    // @return 失败返回nullptr
    static std::unique_ptr<ddiocp> create_instance(s32 thread_count = 0);

    // @param handle: 打开的文件句柄, 该句柄必须是支持iocp且调用者确保有效
    // @return 失败返回false, 使用GetLastError()获取错误码
    virtual bool watch(HANDLE handle, std::size_t key) = 0;

    // 等待端口完成
    virtual ddiocp_notify_type wait(ddiocp_item& item, u32 timeout = INFINITE) = 0;

    // 将任务投递到iocp线程中并唤醒wait去执行任务, 如果没有在waitting状态中,则会在之后的某次唤醒时执行
    // @return 是否成功投递, 如果iocp已经关闭, 则返回false
    virtual bool run_in_iocp_thread(std::function<void()> task) = 0;

    // 关闭 iocp, 如果处于wait状态中, 会导致wait返回false
    // 它的资源会在所有与之相关联的item都释放后才会真正的被释放
    virtual void close() = 0;
};

////////////////////////////////ddiocp_with_dispatcher//////////////////////////////////////////
class ddiocp_with_dispatcher;
class ddiiocp_dispatch
{
    friend ddiocp_with_dispatcher;
public:
    virtual ~ddiiocp_dispatch();

    // 操作完成了
    virtual void on_iocp_complete(const ddiocp_item& item) = 0;

    // iocp 空闲, 可以用来在用户侧做超时检查
    virtual void on_iocp_idle(u64) {};

    // @return 能够绑定到iocp的句柄
    virtual HANDLE get_handle() = 0;

    // 是否是虚拟的dispatch
    // 虚拟的dispatch指的是get_handle(比如放回nullptr,或者一个无效的句柄等)无法被绑定到native iocp上, 但是为了一些目的ddiocp_with_dispatcher::watch依旧会成功
    // 默认值为false
    virtual bool virtual_dispatch();

    ddiocp_with_dispatcher* get_iocp();
    void run_in_next_iocp_loop(std::function<void()> task);

    bool set_timeout(OVERLAPPED* ov, u64 timeout);
    bool remove_timeout(OVERLAPPED* ov);

    // 将这个mutex设置为public让用户可以自由的使用
    std::recursive_mutex m_mutex;

private:
    void set_iocp(ddiocp_with_dispatcher* iocp);
    ddiocp_with_dispatcher* m_iocp = nullptr;
};

class ddiocp_with_dispatcher
{
    ddiocp_with_dispatcher() = default;
public:
    ~ddiocp_with_dispatcher();
    static std::unique_ptr<ddiocp_with_dispatcher> create_instance();

    // @param dispatch: 不能为nullptr, 必须使用智能指针管理, 内部保存的是weak_ptr
    // @return 已经存在, 则返回false, 否则返回true
    bool watch(std::weak_ptr<ddiiocp_dispatch> dispatch);

    // @return 不存在, 则返回nullptr的weaker
    std::weak_ptr<ddiiocp_dispatch> get_weaker(ddiiocp_dispatch* dispatch);

    // @return 不存在, 则返回false, 否则返回true
    // @note dispatch 在删除前必须要调用unwatch来保证不会再有on_iocp_complete回调, 否则会导致访问已经delete的dispatch
    bool unwatch(ddiiocp_dispatch* dispatch);

    // 设置超时, 参数不允许为nullptr, 如果ov已经设置了超时, 则会覆盖之前的超时
    // @note 超时检查间隔为bool dispatch(u32 timeout = INFINITE)函数中设定的参数
    // @return 如果dispatch 没有被watch, 返回false否则返回true
    bool set_timeout(OVERLAPPED* ov, ddiiocp_dispatch* dispatch, u64 timeout);
    void remove_timeout(OVERLAPPED* ov);

    // 返回false表示iocp被关闭了, 此时应该停止pump退出循环
    bool dispatch(u32 timeout = INFINITE);

    // @param task
    // @param dispatch 将task依附到dispatch上, 当准备执行task时候如果dispatch已经被释放了则不执行
    // 将任务投递到iocp线程中并唤醒wait去执行任务, 如果没有在waitting状态中,则会在之后的某次唤醒时执行
    // @return 是否成功投递, 如果iocp已经关闭, 则返回false
    bool run_in_iocp_thread_unsafe(std::function<void()> task);
    bool run_in_iocp_thread(std::function<void()> task, ddiiocp_dispatch* dispatch);

    // 关闭 iocp, 如果处于wait状态中, 会导致wait返回false
    // 它的资源会在所有与之相关联的item都释放后才会真正的被释放
    void close();

private:
    void* m_impl = nullptr;
};

} // namespace NSP_DD
#endif // ddbase_iocp_ddiocp_h_
