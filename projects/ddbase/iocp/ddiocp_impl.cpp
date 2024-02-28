#include "ddbase/stdafx.h"
#include "ddbase/iocp/ddiocp.h"
#include "ddbase/dderror_code.h"

#include <list>
namespace NSP_DD {
#define DDIOCP_CLOSE_TYPE 0 // 关闭iocp
#define DDIOCP_TASK_TYPE 1 // 内部任务的KEY
#define DDIOCP_TASK_OVERLOOP LPOVERLAPPED(1) // 内部任务的overloop

class ddiocp_impl : public ddiocp
{
public:
    bool init(s32 thread_count)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (m_hiocp != NULL) {
            return true;
        }

        m_hiocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, DWORD(thread_count));
        if (!m_hiocp != NULL) {
            return false;
        }

        return true;
    }

    ~ddiocp_impl()
    {
        if (m_hiocp != NULL) {
            ::CloseHandle(m_hiocp);
            m_hiocp = NULL;
        }
    }

    bool watch(HANDLE handle, std::size_t key) override
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (m_hiocp == NULL) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        if (::CreateIoCompletionPort(handle, m_hiocp, (ULONG_PTR)(key), 0) == NULL) {
            return false;
        }

        return true;
    }

    ddiocp_notify_type wait(ddiocp_item& item, u32 timeout /* = INFINITE */) override
    {
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            if (m_hiocp == NULL) {
                dderror_code::set_last_error(dderror_code::init_failure);
                return ddiocp_notify_type::closed;
            }
        }

        BOOL result = ::GetQueuedCompletionStatus(m_hiocp, (LPDWORD) & (item.transferred_number), (PULONG_PTR)&item.key, &item.overlapped, timeout);
        if (item.overlapped == NULL) {
            // 当完成端口未完成返回时候有以下几种情况:
            // 1. ioco被关闭, ::GetLastError() == ERROR_ABANDONED_WAIT_0;
            // 2. 超时返回, ::GetLastError() == WAIT_TIMEOUT;
            // 3. 其它情况按超时处理
            if (::GetLastError() == ERROR_ABANDONED_WAIT_0) {
                return ddiocp_notify_type::closed;
            }

            // ::GetLastError() == WAIT_TIMEOUT and other case.
            return ddiocp_notify_type::timeout;
        }

        // 当完成端口完成时候有以下几种情况:
        // 1. 当OVERLAPPED为DDIOCP_TASK_OVERLOOP时候, 该条消息是由notify()函数调用PostQueuedCompletionStatus发起的内部消息;
        // 2. 当result为TRUE, 正常完成;
        // 3. 当result为FALSE, 异常完成, 比如socket连接断开等;
        if (item.overlapped == DDIOCP_TASK_OVERLOOP) {
            switch (item.transferred_number) {
            case DDIOCP_CLOSE_TYPE: {
                // 这里不需要::CloseHandle(m_iocp);因为这个时候会有已经绑定到该iocp的item还没有被关闭, 所以在这儿close总是会失败, 直接返回false就行了.
                return ddiocp_notify_type::closed;
            }
            case DDIOCP_TASK_TYPE: {
                std::list<std::function<void()>> tasks;
                {
                    std::lock_guard<std::recursive_mutex> lock(m_mutex);
                    m_tasks.swap(tasks);
                }

                for (auto& task : tasks) {
                    task();
                }
                break;
            }
            }
            return ddiocp_notify_type::inner_task_run;
        }
        item.has_error = !result;
        if (item.has_error) {
            item.error_code = dderror_code::get_last_error();
            item.transferred_number = 0;
        }
        return ddiocp_notify_type::complete;
    }

    bool run_in_iocp_thread(std::function<void()> task) override
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_tasks.push_back(task);
        return notify(DDIOCP_TASK_TYPE);
    }

    void close() override
    {
        (void)notify(DDIOCP_CLOSE_TYPE);
    }

    bool notify(s32 type)
    {
        // 这里不用去判断 m_iocp 是否有效，因为如果 m_iocp 无效，那么 PostQueuedCompletionStatus 会失败
        return !!::PostQueuedCompletionStatus(m_hiocp, (DWORD)type, 0, DDIOCP_TASK_OVERLOOP);
    }

private:

    HANDLE m_hiocp = NULL;

    // inner task
    std::recursive_mutex m_mutex;
    std::list<std::function<void()>> m_tasks;
};

////////////////////////////////ddiocp//////////////////////////////////////////
std::unique_ptr<ddiocp> ddiocp::create_instance(s32 thread_count)
{
    auto iocp = std::make_unique<ddiocp_impl>();
    if (iocp == nullptr) {
        return nullptr;
    }

    if (!iocp->init(thread_count)) {
        return nullptr;
    }

    return iocp;
}
} // namespace NSP_DD
