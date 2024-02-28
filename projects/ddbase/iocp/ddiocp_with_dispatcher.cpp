#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include <list>

///////////////////////////////////ddiiocp_dispatch///////////////////////////////////
namespace NSP_DD {
ddiiocp_dispatch::~ddiiocp_dispatch()
{
    if (m_iocp != nullptr) {
        m_iocp->unwatch(this);
    }
}

void ddiiocp_dispatch::set_expire(OVERLAPPED* ov, ddexpire expire /* = ddexpire::never */)
{
    if (m_iocp != nullptr) {
        m_iocp->set_expire(ov, m_key, expire);
    }
}

void ddiiocp_dispatch::transfer_to(const std::weak_ptr<ddiiocp_dispatch>& r)
{
    DDASSERT(m_iocp != nullptr);
    auto iocp = m_iocp;
    iocp->unwatch(this);
    iocp->virtual_watch(r);
}
} // namespace NSP_DD

///////////////////////////////////ddiocp_with_dispatcher///////////////////////////////////
namespace NSP_DD {
ddiocp_with_dispatcher::~ddiocp_with_dispatcher()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    for (auto& it : m_dispatchs) {
        auto sp = it.second.lock();
        sp->on_watched(nullptr, NULL);
    }
}

std::unique_ptr<ddiocp_with_dispatcher> ddiocp_with_dispatcher::create_instance(const ddasync_caller& async_caller /* = nullptr */)
{
    std::unique_ptr<ddiocp_with_dispatcher> iocp(new(std::nothrow) ddiocp_with_dispatcher());
    if (iocp == nullptr) {
        return nullptr;
    }

    if (!iocp->init()) {
        return nullptr;
    }

    iocp->set_idle_interval();
    iocp->m_async_caller = async_caller;
    return iocp;
}

bool ddiocp_with_dispatcher::init()
{
    m_iocp = ddiocp::create_instance();
    return (m_iocp != nullptr);
}

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
        bool timeout = false;
        std::shared_ptr<ddiiocp_dispatch> sp = nullptr;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            m_pending_timeout_ovs.remove_if([&item, &timeout](const ddiocp_ov_timeout& it) {
                timeout = (it.ov == item.overlapped);
                return timeout;
            });

            m_expire_epochs.remove_if([&item](const ddiocp_ov_timeout& it) {
                return it.handle == item.handle;
            });

            auto it = m_dispatchs.find(item.handle);
            if (it != m_dispatchs.end()) {
                sp = it->second.lock();
                if (sp == nullptr) {
                    m_dispatchs.erase(it);
                }
            }
        }

        if (sp != nullptr) {
            ddmaybe_async_call(m_async_caller, [timeout, item, sp]() {
                if (!timeout) {
                    sp->on_iocp_complete_v0(item);
                } else {
                    sp->on_timeout(item.overlapped);
                }
            });
        } else {
            ::CancelIoEx(item.handle, item.overlapped);
        }
    }
    idle();
    return true;
}

bool ddiocp_with_dispatcher::notify_close()
{
    DDASSERT(m_iocp != nullptr);
    return m_iocp->notify_close();
}

bool ddiocp_with_dispatcher::watch(const std::weak_ptr<ddiiocp_dispatch>& dispatch)
{
    DDASSERT(m_iocp != nullptr);
    auto sp = dispatch.lock();
    if (sp == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return false;
    }

    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    HANDLE handle = sp->get_handle();
    if (m_dispatchs.find(handle) != m_dispatchs.end()) {
        return true;
    }

    if (m_iocp->watch(handle)) {
        m_dispatchs[handle] = dispatch;
        sp->on_watched(this, handle);
        return true;
    }
    return false;
}

bool ddiocp_with_dispatcher::virtual_watch(const std::weak_ptr<ddiiocp_dispatch>& dispatch)
{
    DDASSERT(m_iocp != nullptr);
    auto sp = dispatch.lock();
    if (sp == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return false;
    }

    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    HANDLE handle = sp->get_handle();
    if (m_dispatchs.find(handle) == m_dispatchs.end()) {
        m_dispatchs[handle] = dispatch;
        sp->on_watched(this, handle);
    }
    return true;
}

std::weak_ptr<ddiiocp_dispatch> ddiocp_with_dispatcher::get_weaker(ddiiocp_dispatch* dispatch)
{
    DDASSERT(m_iocp != nullptr);
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    auto it = m_dispatchs.find(dispatch->m_key);
    if (it != m_dispatchs.end()) {
        return it->second;
    }
    return std::weak_ptr<ddiiocp_dispatch>();
}

void ddiocp_with_dispatcher::unwatch(ddiiocp_dispatch* dispatch)
{
    DDASSERT(m_iocp != nullptr);
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    HANDLE handle = dispatch->m_key;
    dispatch->m_key = NULL;
    dispatch->on_watched(nullptr, NULL);
    m_dispatchs.erase(handle);
    remove_timeout_on_handle(handle);
}

bool ddiocp_with_dispatcher::is_run_in_iocp_thread() const
{
    DDASSERT(m_iocp != nullptr);
    return m_iocp->is_run_in_iocp_thread();
}

/// task
void ddiocp_with_dispatcher::push_task(const std::function<void()>& task)
{
    DDASSERT(m_iocp != nullptr);
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_tasks.push_back(task);
    m_iocp->notify();
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

/// idle
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

void ddiocp_with_dispatcher::idle()
{
    u64 epoch = ddtime::now_ms();
    if (epoch <= m_pre_idle_epoch + get_idle_interval()) {
        return;
    }

    m_pre_idle_epoch = epoch;
    std::vector<std::weak_ptr<ddiiocp_dispatch>> dispatchs;
    dispatchs.reserve((m_dispatchs.size()));
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        for (auto it : m_dispatchs) {
            DDASSERT(it.second.lock() != nullptr);
            dispatchs.emplace_back(it.second);
        }
    }

    for (auto it : dispatchs) {
        auto sp = it.lock();
        if (sp != nullptr) {
            sp->on_idle(m_pre_idle_epoch);
        }
    }

    handle_timeout_on_idle(m_pre_idle_epoch);
}

/// timeout
void ddiocp_with_dispatcher::handle_timeout_on_idle(u64 now)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    for (auto it = m_expire_epochs.begin(); it != m_expire_epochs.end();) {
        if (now <= it->expire.epoch) {
            ++it;
        } else {
            // cancle the IO and wait for the iocp complete, at that time
            // we think the ov really timeout.
            ::CancelIoEx(it->handle, it->ov);
            m_pending_timeout_ovs.push_back(*it);
            it = m_expire_epochs.erase(it);
        }
    }
}

void ddiocp_with_dispatcher::set_expire(OVERLAPPED* ov, HANDLE handle, ddexpire expire /* = ddexpire::never */)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    for (auto it = m_expire_epochs.begin(); it != m_expire_epochs.end(); ++it) {
        if (it->ov == ov) {
            if (expire == ddexpire::never) {
                m_expire_epochs.erase(it);
            } else {
                it->expire = expire;
            }
            return;
        }
    }

    if (expire != ddexpire::never) {
        m_expire_epochs.push_back({ expire, ov, handle });
    }
}

void ddiocp_with_dispatcher::remove_timeout_on_handle(HANDLE handle)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_expire_epochs.remove_if([handle](const ddiocp_ov_timeout& it) {
        return it.handle == handle;
    });

    m_pending_timeout_ovs.remove_if([handle](const ddiocp_ov_timeout& it) {
        return it.handle == handle;
    });
}
} // namespace NSP_DD
