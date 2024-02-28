#include "ddbase/stdafx.h"
#include "ddbase/iocp/ddiocp.h"
#include "ddbase/dderror_code.h"

#include <list>
namespace NSP_DD {
#define DDIOCP_TASK_OVERLOOP LPOVERLAPPED(1) // 内部任务的overloop

std::unique_ptr<ddiocp> ddiocp::create_instance(s32 thread_count)
{
    std::unique_ptr<ddiocp> iocp(new(std::nothrow) ddiocp());
    if (iocp == nullptr) {
        return nullptr;
    }

    if (!iocp->init(thread_count)) {
        return nullptr;
    }

    return iocp;
}

bool ddiocp::init(s32 thread_count)
{
    DDASSERT(m_hiocp == NULL);
    m_hiocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, DWORD(thread_count));
    if (m_hiocp == NULL) {
        return false;
    }

    return true;
}

ddiocp::~ddiocp()
{
    if (m_hiocp != NULL) {
        ::CloseHandle(m_hiocp);
        m_hiocp = NULL;
    }
}

bool ddiocp::watch(HANDLE handle)
{
    DDASSERT(m_hiocp != NULL);
    if (::CreateIoCompletionPort(handle, m_hiocp, (ULONG_PTR)(handle), 0) == NULL) {
        return false;
    }

    return true;
}

ddiocp_notify_type ddiocp::wait(ddiocp_item& item, u32 timeout /* = INFINITE */)
{
    DDASSERT(m_hiocp != NULL);
    if (m_iocp_thread_id == 0) {
        m_iocp_thread_id = ::GetCurrentThreadId();
    }
    DDASSERT_FMTW(m_iocp_thread_id == ::GetCurrentThreadId(), L"the wait function can run in only one thread");
    BOOL result = ::GetQueuedCompletionStatus(m_hiocp, (LPDWORD)&(item.transferred_number), (PULONG_PTR)&item.handle, &item.overlapped, timeout);
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
        return (ddiocp_notify_type)item.transferred_number;
    }
    item.has_error = !result;
    if (item.has_error) {
        item.error_code = dderror_code::get_last_error();
        item.transferred_number = 0;
    } else {
        (void)::GetOverlappedResult(item.handle, item.overlapped, (LPDWORD)(&item.transferred_number), true);
    }
    return ddiocp_notify_type::complete;
}

bool ddiocp::is_run_in_iocp_thread() const
{
    return m_iocp_thread_id == ::GetCurrentThreadId();
}

bool ddiocp::notify_inner(ddiocp_notify_type type)
{
    DDASSERT(m_hiocp != NULL);
    return !!::PostQueuedCompletionStatus(m_hiocp, (DWORD)type, 0, DDIOCP_TASK_OVERLOOP);
}

bool ddiocp::notify()
{
    return notify_inner(ddiocp_notify_type::notify);
}

bool ddiocp::notify_close()
{
    return notify_inner(ddiocp_notify_type::closed);
}

} // namespace NSP_DD
