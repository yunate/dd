#ifndef ddbase_iocp_ddiocp_h_
#define ddbase_iocp_ddiocp_h_
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include <windows.h>

namespace NSP_DD {
struct ddiocp_item
{
    // 如果为true, 说明某个端口完成了, 但是有错误, 比如连接断开等
    bool has_error = false;
    DWORD error_code = 0;
    u32 transferred_number = 0;
    HANDLE handle = NULL;
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

    // 主动唤醒, 可以在这个时候执行 task
    notify
};

class ddiocp
{
    ddiocp() = default;
public:
    virtual ~ddiocp();

    // 创建iocp
    // @param thread_count: 线程数, 0表示使用cpu核心数
    // @return 失败返回 nullptr
    static std::unique_ptr<ddiocp> create_instance(s32 thread_count = 0);

    // @param handle: 打开的文件句柄, 该句柄必须是支持iocp且调用者确保有效
    // @return 失败返回false, 使用GetLastError()获取错误码
    bool watch(HANDLE handle);

    // 等待端口完成
    ddiocp_notify_type wait(ddiocp_item& item, u32 timeout = INFINITE);

    // 唤醒wait
    bool notify();

    // 关闭 iocp, 如果处于wait状态中, 会导致wait返回false
    // 它的资源会在所有与之相关联的item都释放后才会真正的被释放
    bool notify_close();

    bool is_run_in_iocp_thread() const;

private:
    bool notify_inner(ddiocp_notify_type type);
    bool init(s32 thread_count);
    HANDLE m_hiocp = NULL;
    u64 m_iocp_thread_id = 0;
};
} // namespace NSP_DD
#endif // ddbase_iocp_ddiocp_h_
