#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include <list>

///////////////////////////////////ddiocp_timeout_dispatch///////////////////////////////////
namespace NSP_DD {
void ddiocp_timeout_dispatch::set_timeout(OVERLAPPED* ov, u64 timeout /* = INFINITE */)
{
    auto set_timeout_func = [this, ov, timeout]() {
        for (auto it = m_timeouts.begin(); it != m_timeouts.end(); ++it) {
            if (it->ov == ov) {
                if (timeout == INFINITE) {
                    m_timeouts.erase(it);
                } else {
                    it->expire_epoch = ddtime::now_ms() + timeout;
                }
                return;
            }
        }

        if (timeout != INFINITE) {
            m_timeouts.push_back({ ov, ddtime::now_ms() + timeout });
        }
    };

    auto iocp = get_iocp();
    if (iocp == nullptr || iocp->is_run_in_iocp_thread()) {
        // if the iocp is nullptr, it means it has not been watch.
        set_timeout_func();
    } else {
        iocp->push_task(set_timeout_func);
    }
}

void ddiocp_timeout_dispatch::on_idle(u64 epoch)
{
    for (auto it = m_timeouts.begin(); it != m_timeouts.end();) {
        if (epoch <= it->expire_epoch) {
            ++it;
        } else {
            // cancle the IO and wait for the iocp complete, at that time
            // we think the ov really timeout.
            ::CancelIoEx(get_handle(), it->ov);
            m_pending_timeout_ovs.push_back(it->ov);
            will_timeout(it->ov);
            it = m_timeouts.erase(it);
        }
    }
};

void ddiocp_timeout_dispatch::on_iocp_complete_v0(const ddiocp_item& item)
{
    for (auto it = m_pending_timeout_ovs.begin(); it != m_pending_timeout_ovs.end(); ++it) {
        if (item.overlapped == *it) {
            m_pending_timeout_ovs.erase(it);
            on_timeout(item.overlapped);
            return;
        }
    }

    for (auto it = m_timeouts.begin(); it != m_timeouts.end(); ++it) {
        if (item.overlapped == it->ov) {
            m_timeouts.erase(it);
            break;
        }
    }
    on_iocp_complete_v1(item);
}
}

///////////////////////////////////ddiocp_io_dispatch///////////////////////////////////
namespace NSP_DD {
ddiocp_io_dispatch::~ddiocp_io_dispatch()
{
}

void ddiocp_io_dispatch::ddio_impl::run_inner(
    ddiocp_io_dispatch* dispatch,
    void* buff, s32 buff_size,
    const std::function<void(bool successful, s32 byte)>& callback,
    u64 experid_epoch,
    bool is_write)
{
    DDASSERT_FMTW(m_callback == nullptr, L"the next-call of write()/read() function must wait for the compleation of pre-call.");
    if (m_callback != nullptr) {
        callback(false, 0);
        return;
    }

    if (buff == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        callback(false, 0);
        return;
    }

    auto handle = dispatch->get_handle();
    if (handle == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        callback(false, 0);
        return;
    }

    if (experid_epoch != INFINITE) {
        u64 now = ddtime::now_ms();
        if (experid_epoch < now) {
            dderror_code::set_last_error(dderror_code::time_out);
            callback(false, 0);
            return;
        }

        dispatch->set_timeout(&m_ov, experid_epoch - now);
    }

    BOOL result = FALSE;
    if (is_write) {
        result = ::WriteFile((HANDLE)handle, buff, (DWORD)buff_size, NULL, &m_ov);
    } else {
        result = ::ReadFile((HANDLE)handle, buff, (DWORD)buff_size, NULL, &m_ov);
    }

    if (!result && ::GetLastError() != ERROR_IO_PENDING) {
        dispatch->set_timeout(&m_ov, INFINITE);
        callback(false, 0);
        return;
    }
    m_callback = callback;
}

void ddiocp_io_dispatch::ddio_impl::run(ddiocp_io_dispatch* dispatch,
    void* buff, s32 buff_size,
    const std::function<void(bool successful, s32 byte)>& callback,
    u64 timeout,
    bool is_write)
{
    DDASSERT_FMTW(callback != nullptr, L"callback cannot be nullptr.");
    auto iocp = dispatch->get_iocp();
    DDASSERT_FMTW(iocp != nullptr, L"should call ddiocp_with_dispatcher::watch() first.");
    u64 experid_epoch = INFINITE;
    if (timeout != INFINITE) {
        experid_epoch = ddtime::now_ms() + timeout;
    }

    if (iocp->is_run_in_iocp_thread()) {
        run_inner(dispatch, buff, buff_size, callback, experid_epoch, is_write);
    } else {
        iocp->push_task([this, dispatch, buff, buff_size, callback, experid_epoch, is_write]() {
            run_inner(dispatch, buff, buff_size, callback, experid_epoch, is_write);
        });
    }
}

bool ddiocp_io_dispatch::ddio_impl::on_iocp_complete(const ddiocp_item& item)
{
    if (item.overlapped == &m_ov) {
        dderror_code::set_last_error(item.error_code);
        callback(!item.has_error, item.transferred_number);
        return true;
    }

    return false;
}

void ddiocp_io_dispatch::ddio_impl::callback(bool successful, s32 byte)
{
    DDASSERT(m_callback != nullptr);
    std::function<void(bool successful, s32 byte)> callback = std::move(m_callback);
    callback(successful, byte);
}

bool ddiocp_io_dispatch::ddio_impl::on_timeout(OVERLAPPED* ov)
{
    if (ov != &m_ov) {
        return false;
    }

    dderror_code::set_last_error(dderror_code::time_out);
    callback(false, 0);
    return true;
}

void ddiocp_io_dispatch::write(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, u64 timeout /* = INFINITE */)
{
    m_write_impl.run(this, buff, buff_size, callback, timeout, true);
}

void ddiocp_io_dispatch::read(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, u64 timeout /* = INFINITE */)
{
    m_read_impl.run(this, buff, buff_size, callback, timeout, false);
}

void ddiocp_io_dispatch::on_timeout(OVERLAPPED* ov)
{
    dderror_code::set_last_error(dderror_code::time_out);
    if (m_read_impl.on_timeout(ov)) {
        return;
    }

    if (m_write_impl.on_timeout(ov)) {
        return;
    }
}

void ddiocp_io_dispatch::on_iocp_complete_v1(const ddiocp_item& item)
{
    if (m_read_impl.on_iocp_complete(item)) {
        return;
    }

    if (m_write_impl.on_iocp_complete(item)) {
        return;
    }

    on_iocp_complete_v2(item);
}
}
///////////////////////////////////ddiocp_with_dispatcher///////////////////////////////////
namespace NSP_DD {
std::unique_ptr<ddiocp_with_dispatcher> ddiocp_with_dispatcher::create_instance()
{
    std::unique_ptr<ddiocp_with_dispatcher> iocp(new(std::nothrow) ddiocp_with_dispatcher());
    if (iocp == nullptr) {
        return nullptr;
    }

    if (!iocp->init()) {
        return nullptr;
    }

    iocp->set_idle_interval();
    return iocp;
}

bool ddiocp_with_dispatcher::init()
{
    m_iocp = ddiocp::create_instance();
    return (m_iocp != nullptr);
}

bool ddiocp_with_dispatcher::notify_close()
{
    DDASSERT(m_iocp != nullptr);
    return m_iocp->notify_close();
}

bool ddiocp_with_dispatcher::watch(ddiiocp_dispatch* dispatch)
{
    DDASSERT(m_iocp != nullptr);
    if (dispatch == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return false;
    }

    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    HANDLE handle = dispatch->get_handle();
    if (m_dispatchs.find(handle) != m_dispatchs.end()) {
        return true;
    }

    if (m_iocp->watch(handle)) {
        m_dispatchs[handle] = dispatch;
        dispatch->set_iocp(this);
        return true;
    }
    return false;
}

bool ddiocp_with_dispatcher::unwatch(ddiiocp_dispatch* dispatch)
{
    DDASSERT(m_iocp != nullptr);
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_dispatchs.erase(dispatch);
    dispatch->set_iocp(nullptr);
    return true;
}

// 返回false表示iocp被关闭了, 此时应该停止pump退出循环
bool ddiocp_with_dispatcher::dispatch()
{
    DDASSERT(m_iocp != nullptr);
    ddiocp_item item;
    ddiocp_notify_type type = m_iocp->wait(item, (u32)get_idle_interval());

    // we wish the tasks always have a high priority.
    run_task();

    if (type == ddiocp_notify_type::closed) {
        return false;
    } else if (type == ddiocp_notify_type::complete) {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        auto it = m_dispatchs.find(item.handle);
        if (it != m_dispatchs.end()) {
            it->second->on_iocp_complete_v0(item);
        }
    }

    idle();
    return true;
}

void ddiocp_with_dispatcher::run_task()
{
    std::list<std::function<void()>> tasks;
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (m_tasks.empty()) {
            return;
        } else {
            tasks = std::move(m_tasks);
        }
    }

    for (auto& task : tasks) {
        task();
    }
}

void ddiocp_with_dispatcher::push_task(const std::function<void()>& task)
{
    DDASSERT(m_iocp != nullptr);
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_tasks.push_back(task);
    m_iocp->notify();
}

u64 ddiocp_with_dispatcher::get_idle_interval()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_idle_interval;
}

void ddiocp_with_dispatcher::set_idle_interval(u64 interval /* = 5000 */ )
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_idle_interval = interval;
    if (m_idle_interval >= 5000) {
        m_idle_interval = 5000;
    } else if (m_idle_interval < 1000) {
        m_idle_interval = 1000;
    }
}

bool ddiocp_with_dispatcher::is_run_in_iocp_thread() const
{
    DDASSERT(m_iocp != nullptr);
    return m_iocp->is_run_in_iocp_thread();
}

void ddiocp_with_dispatcher::idle()
{
    u64 epoch = ddtime::now_ms();
    if (epoch <= m_pre_idle_epoch + get_idle_interval()) {
        return;
    }

    m_pre_idle_epoch = epoch;
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    std::map<HANDLE, ddiiocp_dispatch*> dispatchs = m_dispatchs;
    for (auto it : dispatchs) {
        it.second->on_idle(m_pre_idle_epoch);
    }
}
} // namespace NSP_DD
