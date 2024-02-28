#include "ddbase/stdafx.h"
#include "ddbase/iocp/ddiocp.h"
#include "ddbase/dderror_code.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddtimer.h"

#include <memory>
#include <map>
#include <list>

namespace NSP_DD {

////////////////////////////////ddiocp_with_dispatcher_impl//////////////////////////////////////////
class ddiocp_with_dispatcher_impl
{
public:
    bool init()
    {
        m_iocp = ddiocp::create_instance();
        return (m_iocp != nullptr);
    }

    ~ddiocp_with_dispatcher_impl()
    {
        close();
    }

    bool watch(std::weak_ptr<ddiiocp_dispatch> weak)
    {
        DDASSERT(m_iocp != nullptr);
        auto dispatch = weak.lock();
        if (dispatch == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            return false;
        }

        HANDLE handle = dispatch->get_handle();
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (m_dispatchs.find(dispatch.get()) != m_dispatchs.end()) {
            dderror_code::set_last_error(dderror_code::key_exist);
            return false;
        }

        if (!dispatch->virtual_dispatch()) {
            if (!m_iocp->watch(handle, (std::size_t)(dispatch.get()))) {
                return false;
            }
        }

        m_dispatchs[dispatch.get()] = weak;
        return true;
    }

    std::weak_ptr<ddiiocp_dispatch> get_weaker(ddiiocp_dispatch* dispatch)
    {
        DDASSERT(m_iocp != nullptr);
        if (dispatch == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            return std::weak_ptr<ddiiocp_dispatch>();
        }

        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        auto it = m_dispatchs.find(dispatch);
        if (it != m_dispatchs.end()) {
            return it->second;
        }

        return std::weak_ptr<ddiiocp_dispatch>();
    }

    bool unwatch(ddiiocp_dispatch* dispatch)
    {
        DDASSERT(m_iocp != nullptr);
        if (dispatch == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            return false;
        }

        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (m_dispatchs.find(dispatch) == m_dispatchs.end()) {
            dderror_code::set_last_error(dderror_code::key_not_find);
            return false;
        }

        m_dispatchs.erase(dispatch);

        // remove the timeout
        remove_timeout_inner(dispatch);
        return true;
    }

    bool set_timeout(OVERLAPPED* ov, ddiiocp_dispatch* dispatch, u64 timeout)
    {
        DDASSERT(m_iocp != nullptr);
        DDASSERT(ov != nullptr);
        DDASSERT(dispatch != nullptr);
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (m_dispatchs.find(dispatch) == m_dispatchs.end()) {
            return false;
        }

        if (timeout == INFINITE) {
            for (auto it = m_expire_epochs.begin(); it != m_expire_epochs.end(); ++it) {
                if ((*it).ov == ov) {
                    // remove the timeout
                    m_expire_epochs.erase(it);
                    break;
                }
            }
            return true;
        }

        // if the timeout had been set, update the timeout.
        for (auto& it : m_expire_epochs) {
            if (it.ov == ov) {
                it.epoch = timeout + ddtime::now_ms();
                it.dispatch = dispatch;
                return true;
            }
        }

        // insert the timout
        m_expire_epochs.push_back(expire_epoch{ dispatch, ov, timeout + ddtime::now_ms()});
        return true;
    }

    void remove_timeout(OVERLAPPED* ov)
    {
        DDASSERT(m_iocp != nullptr);
        DDASSERT(ov != nullptr);
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        for (auto it = m_expire_epochs.begin(); it != m_expire_epochs.end(); ++it) {
            if ((*it).ov == ov) {
                // remove the timeout
                m_expire_epochs.erase(it);
                break;
            }
        }
    }

    bool dispatch(u32 timeout /* = INFINITE */)
    {
        DDASSERT(m_iocp != nullptr);
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            m_timeout = timeout;
        }

        ddiocp_item item;
        ddiocp_notify_type type = m_iocp->wait(item, timeout);
        if (type == ddiocp_notify_type::closed) {
            return false;
        }

        std::shared_ptr<ddiiocp_dispatch> spdispatch;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            if (type == ddiocp_notify_type::complete) {
                ddiiocp_dispatch* dispatch = (ddiiocp_dispatch*)item.key;
                if (m_dispatchs.find(dispatch) != m_dispatchs.end()) {
                    spdispatch = m_dispatchs[dispatch].lock();
                }
            }
        }

        if (spdispatch != nullptr) {
            spdispatch->on_iocp_complete(item);
            spdispatch.reset();
        }

        idle();
        return true;
    }

    void idle()
    {
        u64 epoch = ddtime::now_ms();
        std::map<ddiiocp_dispatch*, std::weak_ptr<ddiiocp_dispatch>> dispatchs;
        std::list<expire_epoch> timeout_epochs;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            u64 timeout = m_timeout;
            if (timeout == INFINITE) {
                timeout = 50000;
            }

            if (epoch - m_pre_idle_epoch < timeout) {
                return;
            }

            m_pre_idle_epoch = epoch;

            dispatchs = m_dispatchs;

            // find the timeout epoch
            auto it = m_expire_epochs.begin();
            while (it != m_expire_epochs.end()) {
                if ((*it).epoch > epoch) {
                    ++it;
                    continue;
                }

                timeout_epochs.push_back(*it);
                it = m_expire_epochs.erase(it);
            }
        }

        // tell the dispatchs the idle epoch.
        for (auto it : dispatchs) {
            auto dispatch = it.second.lock();
            if (dispatch == nullptr) {
                continue;
            }

            dispatch->on_iocp_idle(epoch);
        }

        // timeout epoch
        for (auto it : timeout_epochs) {
            std::shared_ptr<ddiiocp_dispatch> dispatch;
            {
                std::lock_guard<std::recursive_mutex> locker(m_mutex);
                auto find = m_dispatchs.find(it.dispatch);
                if (find == m_dispatchs.end()) {
                    continue;
                }

                dispatch = (find->second).lock();
            }

            if (dispatch != nullptr) {
                // cancle the io, and the iocp will complete with the error.
                // so here just cancle the IO and do not call the on_iocp_complete function.
                (void)::CancelIoEx(dispatch->get_handle(), it.ov);
            }
        }
    }

    bool run_in_iocp_thread_unsafe(std::function<void()> task)
    {
        DDASSERT(m_iocp != nullptr);
        return m_iocp->run_in_iocp_thread(task);
    }

    bool run_in_iocp_thread(std::function<void()> task, ddiiocp_dispatch* dispatch)
    {
        DDASSERT(m_iocp != nullptr);
        return m_iocp->run_in_iocp_thread([task, dispatch, this](){
            std::shared_ptr<ddiiocp_dispatch> holder;
            {
                std::lock_guard<std::recursive_mutex> locker(m_mutex);
                auto it = m_dispatchs.find(dispatch);
                if (it == m_dispatchs.end()) {
                    return;
                }

                holder = (it->second).lock();
                if (dispatch == nullptr) {
                    return;
                }
            }

            task();
        });
    }

    void close()
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        m_expire_epochs.clear();
        m_dispatchs.clear();
        m_iocp->close();
    }

private:
    void remove_timeout_inner(ddiiocp_dispatch* dispatch)
    {
        auto it = m_expire_epochs.begin();
        while (it != m_expire_epochs.end()) {
            if ((*it).dispatch == dispatch) {
                it = m_expire_epochs.erase(it);
                continue;
            }
            ++it;
        }
    }

    std::recursive_mutex m_mutex;
    std::unique_ptr<ddiocp> m_iocp;

    // 不直接使用std::weak_ptr<ddiiocp_dispatch>当作key
    std::map<ddiiocp_dispatch*, std::weak_ptr<ddiiocp_dispatch>> m_dispatchs;
    struct expire_epoch
    {
        ddiiocp_dispatch* dispatch;
        OVERLAPPED* ov;
        u64 epoch;
    };
    std::list<expire_epoch> m_expire_epochs;
    u64 m_timeout = 0;
    u64 m_pre_idle_epoch = 0;
};

////////////////////////////////ddiiocp_dispatch//////////////////////////////////////////
ddiiocp_dispatch::~ddiiocp_dispatch()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_iocp != nullptr) {
        m_iocp->unwatch(this);
    }
}

ddiocp_with_dispatcher* ddiiocp_dispatch::get_iocp()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    return m_iocp;
}

bool ddiiocp_dispatch::virtual_dispatch()
{
    return false;
}

void ddiiocp_dispatch::run_in_next_iocp_loop(std::function<void()> task)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_iocp != nullptr) {
        m_iocp->run_in_iocp_thread([task]() {
            task();
        }, this);
    }
}

bool ddiiocp_dispatch::set_timeout(OVERLAPPED* ov, u64 timeout)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_iocp == nullptr) {
        return false;
    }
    return m_iocp->set_timeout(ov, this, timeout);
}

bool ddiiocp_dispatch::remove_timeout(OVERLAPPED* ov)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_iocp == nullptr) {
        return false;
    }
    m_iocp->remove_timeout(ov);
    return true;
}

void ddiiocp_dispatch::set_iocp(ddiocp_with_dispatcher* iocp)
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_iocp = iocp;
}

///////////////////////////////ddiocp_with_dispatcher//////////////////////////////////////////
std::unique_ptr<ddiocp_with_dispatcher> ddiocp_with_dispatcher::create_instance()
{
    auto iocp = std::unique_ptr<ddiocp_with_dispatcher>(new (std::nothrow)ddiocp_with_dispatcher());
    if (iocp == nullptr) {
        return nullptr;
    }

    auto iocp_impl = std::unique_ptr<ddiocp_with_dispatcher_impl>(new (std::nothrow)ddiocp_with_dispatcher_impl());
    if (iocp == nullptr) {
        return nullptr;
    }

    if (!iocp_impl->init()) {
        return nullptr;
    }

    iocp->m_impl = iocp_impl.release();
    return iocp;
}

ddiocp_with_dispatcher::~ddiocp_with_dispatcher()
{
    delete ((ddiocp_with_dispatcher_impl*)m_impl);
}

bool ddiocp_with_dispatcher::watch(std::weak_ptr<ddiiocp_dispatch> dispatch)
{
    auto shared = dispatch.lock();
    if (shared == nullptr) {
        return false;
    }
    if (((ddiocp_with_dispatcher_impl*)m_impl)->watch(dispatch)) {
        shared->set_iocp(this);
        return true;
    }
    return false;
}

std::weak_ptr<ddiiocp_dispatch> ddiocp_with_dispatcher::ddiocp_with_dispatcher::get_weaker(ddiiocp_dispatch* dispatch)
{
    return ((ddiocp_with_dispatcher_impl*)m_impl)->get_weaker(dispatch);
}

bool ddiocp_with_dispatcher::unwatch(ddiiocp_dispatch* dispatch)
{
    if (((ddiocp_with_dispatcher_impl*)m_impl)->unwatch(dispatch)) {
        dispatch->set_iocp(nullptr);
        return true;
    }
    return false;
}

bool ddiocp_with_dispatcher::set_timeout(OVERLAPPED* ov, ddiiocp_dispatch* dispatch, u64 timeout)
{
    return ((ddiocp_with_dispatcher_impl*)m_impl)->set_timeout(ov, dispatch, timeout);
}

void ddiocp_with_dispatcher::remove_timeout(OVERLAPPED* ov)
{
    ((ddiocp_with_dispatcher_impl*)m_impl)->remove_timeout(ov);
}

bool ddiocp_with_dispatcher::dispatch(u32 timeout)
{
    return ((ddiocp_with_dispatcher_impl*)m_impl)->dispatch(timeout);
}

bool ddiocp_with_dispatcher::run_in_iocp_thread_unsafe(std::function<void()> task)
{
    return ((ddiocp_with_dispatcher_impl*)m_impl)->run_in_iocp_thread_unsafe(task);
}

bool ddiocp_with_dispatcher::run_in_iocp_thread(std::function<void()> task, ddiiocp_dispatch* dispatch)
{
    return ((ddiocp_with_dispatcher_impl*)m_impl)->run_in_iocp_thread(task, dispatch);
}

void ddiocp_with_dispatcher::close()
{
    ((ddiocp_with_dispatcher_impl*)m_impl)->close();
}

} // namespace NSP_DD
