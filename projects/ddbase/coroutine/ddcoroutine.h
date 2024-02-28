#ifndef ddbase_coroutine_ddcoroutine_h_
#define ddbase_coroutine_ddcoroutine_h_

#include "ddbase/coroutine/ddcoroutine_traits_.hpp"

#if _HAS_CXX20
#include <functional>
#include <atomic>
#include <set>

namespace NSP_DD {
template<class T = void>
class ddcoroutine : public ddcoroutine_traits<T>::ddcoroutine_base
{
public:
    void run()
    {
#ifdef _DEBUG
        set_and_check_called();
#endif
        // step 2. the caller call the run function to resume the initial_suspend which return std::suspend_always
        DDASSERT(m_promise_ctx != nullptr);
        m_promise_ctx->resume();
    }

public:
    // step 2. when the caller call the co_await() it will return a ddcaller_awaiter object,
    // its await_ready return false to call await_suspend to set the caller_handle,
    // and its await_suspend function return self_co_handle to resume the initial_suspend which return std::suspend_always in the caller's code call stack.
    auto operator co_await() const& noexcept
    {
#ifdef _DEBUG
        const_cast<ddcoroutine*>(this)->set_and_check_called();
#endif
        struct ddcaller_awaiter : public ddcoroutine_traits<T>::ddcaller_awaiter_base
        {
            constexpr bool await_ready() const noexcept { return false; }
            auto await_suspend(std::coroutine_handle<> caller_handle) noexcept
            {
                DDASSERT(m_weak_called_ctx != nullptr);
                m_weak_called_ctx->set_caller_co_handle(caller_handle);
                // why use return the co_handle rather than call co_handle.resume() [Symmetric Transfer & Tail-calls]:
                // https://lewissbaker.github.io/2020/05/11/understanding_symmetric_transfer
                // when await_suspend return std::coroutine_handle<>, it will use Tail-calls to avoid stack overflow.
                // m_weak_called_ctx->get_self_co_handle().resume(); // we do not use co_handle.resume().
                return m_weak_called_ctx->get_self_co_handle();
            }
        };
        return ddcaller_awaiter{ m_promise_ctx.get() };
    }

    class ddcoroutine_type : public ddcoroutine_traits<T>::ddcoroutine_type_base
    {
    public:
        // step 1. when the function is called, it will return the ddcoroutine object,
        // and its initial_suspend function return std::suspend_always to suppend before enter the function,
        // the purpose is to have the chance to set the caller_handle before the called function run.
        ddcoroutine<T> get_return_object()
        {
            ddcoroutine<T> tmp(this);
            m_weak_ctx = tmp.m_promise_ctx.get();
            return tmp;
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        // step 3. the coroutine will finish, the ddcoroutine_context will be reset in this function.
        // if the step 2 called co_await(caller_handle is valid), await_suspend will return the caller's coroutine_handle to resume the caller.
        // if the step 2 called run function(caller_handle is not valid), await_suspend will return the noop_coroutine to do nothing.
        auto final_suspend() noexcept
        {
            struct ddfinal_awaiter
            {
                std::coroutine_handle<> caller_handle;
                bool await_ready() const noexcept
                {
                    return false;
                }

                std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept
                {
                    if (caller_handle) {
                        return caller_handle;
                    } else {
                        return std::noop_coroutine();
                    }
                }

                void await_resume() const noexcept {}
            };

            return ddfinal_awaiter{ m_weak_ctx->get_caller_co_handle() };
        }
        void unhandled_exception() {}
    };
    using promise_type = ddcoroutine_type;

    bool operator==(const ddcoroutine& r) const { return r.m_promise_ctx == m_promise_ctx; }
    bool operator<(const ddcoroutine& r) const { return r.m_promise_ctx.get() < m_promise_ctx.get(); }
protected:
    ddcoroutine(promise_type* r)
    {
        m_promise_ctx = std::make_shared<ddcoroutine_context<T>>();
        m_promise_ctx->set_self_co_handle(std::coroutine_handle<promise_type>::from_promise(*r));
    }

#ifdef _DEBUG
    std::shared_ptr<std::atomic_bool> m_is_called{ new std::atomic_bool(false) };
    void set_and_check_called()
    {
        if (m_is_called->exchange(true)) {
            DDASSERT_FMTW(false, L"run() or ddcoroutine::co_await() can be called only once.");
        }
    }
#endif
};

struct ddcoroutine_noop
{
    struct promise_type
    {
        ddcoroutine_noop get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

// get current function's co_handle.
// e.g. std::coroutine_handle<> handle = co_await ddget_current_co_handle();
inline auto ddget_current_co_handle()
{
    struct ddawaiter
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr std::coroutine_handle<> await_resume() const noexcept
        {
            return caller_handle;
        }
        auto await_suspend(std::coroutine_handle<> handle) noexcept
        {
            caller_handle = handle;
            return caller_handle;
        }

        std::coroutine_handle<> caller_handle;
    };

    return ddawaiter{};
}

class ddresume_helper
{
public:
    ddresume_helper(const std::function<void()>& resumer)
        : m_resumer(resumer)
    {
    }

    ~ddresume_helper()
    {
        if (m_lazy) {
            resume();
        }
    }

    inline void resume() const
    {
        if (m_resumer != nullptr) {
#ifdef _DEBUG
            check_and_set_resumed();
#endif
            m_lazy = false;
            m_resumer();
        }
    }

    // resume when destruct
    inline void lazy_resume() const
    {
        m_lazy = true;
    }

private:
#ifdef _DEBUG
    mutable std::shared_ptr<bool> m_resumed;
    void check_and_set_resumed() const
    {
        if (m_resumed == nullptr) {
            m_resumed = std::make_shared<bool>(false);
        }
        DDASSERT(!*m_resumed);
        *m_resumed = true;
    }
#endif
    mutable bool m_lazy = false;
    std::function<void()> m_resumer;
};
using ddco_task = std::function<void(const ddresume_helper& resumer)>;
inline auto ddco_async(const ddco_task& task)
{
    struct ddawaiter
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_resume() const noexcept {}
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept
        {
            if (m_task == nullptr) {
                return caller_handle;
            }

            m_task(ddresume_helper([caller_handle, this]() {
                if (m_flag.exchange(false, std::memory_order_acq_rel)) {
                    // path 2
                    // 1. 该callback是同步的.
                    // 2. 设置m_flag线程竞争成功, 即线程调度的非常快, await_suspend还没返回就已经完成了, 这种情况可以认为也是同步的.
                    // 这两种情况只要让await_suspend返回caller_handle即可.
                } else {
                    // path 1
                    caller_handle.resume();
                }
            }));

            if (m_flag.exchange(false, std::memory_order_acq_rel)) {
                // path 1
                return std::noop_coroutine();
            } else {
                // path 2
                return caller_handle;
            }
        }
        ddco_task m_task = nullptr;
        std::atomic_bool m_flag = true;
    };
    return ddawaiter{ task };
};

// do not use const XXX& r, because the ref of XXX may be released
// when this coroutine resume.
// e.g.:
// ddcoroutine<void> co_async()
// {
//     co_return;
// }
// 
// ddcoroutine<void> test()
// {
//     auto x = NSP_DD::ddcoroutine_from(co_async());
//     // the ddcoroutine which is returned by co_async() had released, if use const XXX& r,
//     // it is an UB.
//     co_await x;
// }
inline ddcoroutine<void> ddcoroutine_from(ddco_task task)
{
    co_await ddco_async(task);
}

template<class T>
inline ddcoroutine<void> ddcoroutine_from(ddcoroutine<T> r)
{
    co_await r;
}

inline ddcoroutine<void> ddcoroutine_from(std::vector<ddcoroutine<void>> r)
{
    std::coroutine_handle<> handle = co_await ddget_current_co_handle();
    std::atomic_bool flag = true;
    std::atomic_size_t remain = r.size();
    auto wrapper_func = [&handle, &remain](const ddcoroutine<void>& co,
        std::atomic_size_t* premain,
        std::coroutine_handle<>* phandle,
        std::atomic_bool& flag) -> ddcoroutine_noop {
        co_await co;
        if (premain->fetch_sub(1) == 1) {
            if (flag.exchange(false, std::memory_order_acq_rel)) {
                // path 2
                // 1. 该callback是同步的.
                // 2. 设置m_flag线程竞争成功, 即线程调度的非常快, await_suspend还没返回就已经完成了, 这种情况可以认为也是同步的.
                // 这两种情况只要让await_suspend返回caller_handle即可.
            } else {
                // path 1
                phandle->resume();
            }
        }
        co_return;
    };

    auto wrapper_all = [&r, &remain, &handle, &wrapper_func, &flag]() {
        for (const auto& it : r) {
            wrapper_func(it, &remain, &handle, flag);
        }
    };

    struct ddawaiter
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_resume() const noexcept {}
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) const noexcept
        {
            wrapper_all();
            if (m_flag.exchange(false, std::memory_order_acq_rel)) {
                // path 1
                return std::noop_coroutine();
            } else {
                // path 2
                return caller_handle;
            }
        }
        std::function<void()> wrapper_all;
        std::atomic_bool& m_flag;
    };
    co_await ddawaiter{ wrapper_all, flag };
}

inline ddcoroutine<void> ddcoroutine_all(const std::vector<ddcoroutine<void>>& r)
{
    return ddcoroutine_from(r);
}

// 该函数允许运行一个没有返回值的ddcoroutine, 而不需要关心其生命周期;
// e.g. 一般的用法
// ddcoroutine<int> test()
// {
//     co_return 3;
// }
// int main()
// {
//     // 需要保证co不会被销毁
//     auto co = test();
//     int x = co_await co;
// }
// 
// e.g. 自动保证生命周期
// ddcoroutine<void> test()
// {
//     co_return;
// }
// int main()
// {
//     ddcoroutine_run(test());
// }
inline void ddcoroutine_run(const ddcoroutine<void>& r)
{
    class ddcoroutine_holder
    {
    public:
        inline void add_coroutine(const std::coroutine_handle<>& r)
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            if (m_holded_handle.find(r) == m_holded_handle.end()) {
                m_holded_handle.insert(r);
            }
        }

        inline void remove_coroutine(const std::coroutine_handle<>& r)
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            if (m_holded_handle.find(r) != m_holded_handle.end()) {
                m_holded_handle.erase(r);
            }
        }

        ~ddcoroutine_holder()
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            for (std::coroutine_handle<> it : m_holded_handle) {
                it.destroy();
            }
            m_holded_handle.clear();
        }
    private:
        std::set<std::coroutine_handle<>> m_holded_handle;
        std::mutex m_mutex;
    };

    static ddcoroutine_holder holder;

    auto inner_func = [](ddcoroutine<void> r) -> ddcoroutine_noop {
        std::coroutine_handle<> handle = co_await ddget_current_co_handle();
        holder.add_coroutine(handle);
        co_await r;
        holder.remove_coroutine(handle);
    };
    inner_func(r);
}
} // namespace NSP_DD
#endif // c20
#endif // ddbase_coroutine_ddcoroutine_h_

