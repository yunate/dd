#ifndef ddbase_coroutine_ddcoroutine_h_
#define ddbase_coroutine_ddcoroutine_h_

#if _HAS_CXX20
#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include <coroutine>
#include <functional>
#include <atomic>
#include <set>
#include <mutex>

namespace NSP_DD {
namespace detail {
class ddcoroutine_any_resumer {
public:
    virtual ~ddcoroutine_any_resumer() {}
    virtual void resume() = 0;
};

enum ddflag_set_point
{
    not_set = 0,
    set_in_co_await = 1,
    set_in_final_suspend = 2,
    set_in_ddcoroutine_any_function = 3,
};

template<class T>
struct ddcoroutine_value { T value{}; };
template<>
struct ddcoroutine_value<void> {};
template<class T>
struct ddcoroutine_context : public ddcoroutine_value<T>
{
    ~ddcoroutine_context()
    {
        if (self_co_handle) {
            self_co_handle.destroy();
        }
    }

    inline bool check_unset_and_set(ddflag_set_point set_point)
    {
        unsigned char expect = ddflag_set_point::not_set;
        return set_point_flag.compare_exchange_strong(expect, (unsigned char)set_point, std::memory_order_acq_rel);
    }

    inline void set_caller_co_handle(void* handle)
    {
#ifdef _DEBUG
        if (handle != nullptr) {
            if (caller_co_handler_set_flag.exchange(true, std::memory_order_acq_rel)) {
                // set_caller_co_handle 被调用多次, 可能是由于co_await多次作用于同一个对象
                DDASSERT(false);
            }
        } else {
            caller_co_handler_set_flag.exchange(false, std::memory_order_acq_rel);
        }
#endif
        caller_co_handle = handle;
    }

#ifdef _DEBUG
    std::atomic_bool caller_co_handler_set_flag = false;
#endif

    std::atomic_uchar set_point_flag = ddflag_set_point::not_set;
    std::coroutine_handle<> self_co_handle = nullptr;
    void* caller_co_handle = nullptr;
};

template<class T>
class ddpromise_type_base
{
public:
    void return_value(T&& v)
    {
        DDASSERT(m_self_ctx != nullptr);
        m_self_ctx->value = v;
    }

    void return_value(const T& v)
    {
        DDASSERT(m_self_ctx != nullptr);
        m_self_ctx->value = v;
    }

protected:
    ddcoroutine_context<T>* m_self_ctx = nullptr;
};

template<>
class ddpromise_type_base<void>
{
public:
    void return_void() {}
protected:
    ddcoroutine_context<void>* m_self_ctx = nullptr;
};
} // namespace detail
} // namespace NSP_DD

namespace NSP_DD {
/**
void do_something() {}
ddcoroutine foo()
{
    do_something();
    co_return;
}

// ==>
void foo_resume(foo_co_context* co_context);
void foo_destroy(foo_co_context* co_context);
struct foo_co_context
{
    ddcoroutine::promise_type promise;
    void (*resume_fn)(foo_co_context*) = &foo_resume;
    void (*destroy_fn)(foo_co_context*) = &foo_destroy;
    int suspend_index = 0;
    std::suspend_never init_suspend;
    std::suspend_never final_suspend;
};

ddcoroutine foo()
{
    // init co context
    foo_co_context * co_context = new foo_co_context();
    auto return_obj = co_context->promise.get_return_object();

    // call foo_resume
    foo_resume(co_context);
    return return_obj;
}

void foo_resume(foo_co_context* co_context)
{
    switch(co_context->suspend_index) {
      case 0: break;
      case 1: goto resume_index_1;
      case 2: goto resume_index_2;
    }

    // co_await init_suspend
    {
        ++co_context->suspend_index;
        co_context->init_suspend = co_context->promise.initial_suspend();
        if(!co_context->init_suspend.await_ready()) {
          co_context->init_suspend.await_suspend(std::coroutine_handle<ddcoroutine::promise_type>::from_address(co_context->promise));
          return;
        }

      resume_index_1:
        co_context->init_suspend.await_resume();
    }

    do_something();

    // co_return
    co_context->promise.return_void();

    // co_await final_suspend
    {
        ++co_context->suspend_index;
        co_context->final_suspend = co_context->promise.final_suspend();
        if(!co_context->final_suspend.await_ready()) {
          co_context->final_suspend.await_suspend(std::coroutine_handle<ddcoroutine::promise_type>::from_address(co_context->promise));
          return;
        }

      resume_index_2:
        co_context->destroy_fn(co_context);
    }
}

void foo_destroy(foo_co_context* co_context)
{
  delete co_context;
}
*/
template<class T = void>
class ddcoroutine
{
public:
    const std::add_lvalue_reference_t<T> get_value()
    {
        if constexpr (!std::is_same_v<T, void>) {
            DDASSERT(m_ctx != nullptr);
            return m_ctx->value;
        }
    }

    auto operator co_await() const& noexcept
    {
        struct ddawaiter
        {
            constexpr bool await_ready() const noexcept { return false; }
            bool await_suspend(std::coroutine_handle<> caller_handle) noexcept
            {
                DDASSERT(m_self_ctx != nullptr);
                m_self_ctx->set_caller_co_handle(caller_handle.address());
                if (m_self_ctx->check_unset_and_set(detail::ddflag_set_point::set_in_co_await)) {
                    // path 1(异步)final_suspend 还没有运行, 考虑在此加入::Sleep(1000);
                    // 此时, caller_handle 可能final_suspend中被唤醒, 这种情况下ddcoroutine可能已经被析构了
                    // 再使用任何该类的成员都是未定义行为, 包括返回std::coroutine_handle<>, 所以本函数选择返回bool类型而不是std::coroutine_handle<>类型
                    return true;
                } else {
                    // path 2 (同步), final_suspend 已经运行了, 所以不需要在suspend caller了.
                    return false;
                }
            }

            const std::add_lvalue_reference_t<T> await_resume() const noexcept
            {
                if constexpr (!std::is_same_v<T, void>) {
                    DDASSERT(m_self_ctx != nullptr);
                    return m_self_ctx->value;
                }
            }

            detail::ddcoroutine_context<T>* m_self_ctx = nullptr;
        };
        return ddawaiter{ m_ctx.get() };
    }

    class promise_type : public detail::ddpromise_type_base<T>
    {
        using detail::ddpromise_type_base<T>::m_self_ctx;
    public:
        ddcoroutine<T> get_return_object()
        {
            ddcoroutine<T> tmp(std::coroutine_handle<promise_type>::from_promise(*this));
            m_self_ctx = tmp.m_ctx.get();
            return tmp;
        }

        std::suspend_never initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept
        {
            struct ddawaiter
            {
                detail::ddcoroutine_context<T>* ctx;
                bool await_ready() const noexcept
                {
                    return false;
                }

                // final_suspend 的 await_suspend 返回void, coroutine_handle::destroy()不会自动调用,
                // 我们在detail::ddcoroutine_context的析构函数中调用coroutine_handle::destroy(), 这是
                // 非常有必要的, 因为如果我们返回bool类型false的话, 那么有可能协程对象在co_await之前就销毁了
                void await_suspend(std::coroutine_handle<>) noexcept
                {
                    if (ctx->check_unset_and_set(detail::ddflag_set_point::set_in_final_suspend)) {
                        // path 2
                        // 有以下两种情况, 都可以认为也是同步的
                        // 1. 同步的.
                        // 2. 线程调度的非常快, await_suspend还没返回就已经完成了
                        return;
                    } else {
                        // path 1
                        if (!ctx->caller_co_handle) {
                            return;
                        }
                        if (ctx->set_point_flag == detail::ddflag_set_point::set_in_co_await) {
                            std::coroutine_handle<void>::from_address(ctx->caller_co_handle).resume();
                            return;
                        } else if (ctx->set_point_flag == detail::ddflag_set_point::set_in_ddcoroutine_any_function) {
                            detail::ddcoroutine_any_resumer* resumer = (detail::ddcoroutine_any_resumer*)(ctx->caller_co_handle);
                            ctx->set_caller_co_handle(nullptr);
                            resumer->resume();
                            delete resumer;
                            return;
                        }
                    }
                }

                void await_resume() const noexcept {}
            };

            return ddawaiter{ m_self_ctx };
        }
        void unhandled_exception() {}
    };

    bool operator==(const ddcoroutine& r) const { return r.m_ctx == m_ctx; }
    bool operator<(const ddcoroutine& r) const { return r.m_ctx.get() < m_ctx.get(); }
protected:
    ddcoroutine(std::coroutine_handle<> self_handle)
    {
        m_ctx = std::make_shared<detail::ddcoroutine_context<T>>();
        m_ctx->self_co_handle = self_handle;
    }

    std::shared_ptr<detail::ddcoroutine_context<T>> m_ctx = nullptr;
};

template<class T = void>
ddcoroutine<T> ddcoroutine_empty()
{
    static_assert(std::is_same_v<T, void>, "T must be void");
    co_return;
}

template<class T>
ddcoroutine<T> ddcoroutine_empty(const T& v)
{
    co_return v;
}

// DDCO_REF is a design convention.
// If a coroutine function parameter is prefixed with DDCO_REF, the caller must ensure that the parameter is not released before the coroutine function finishes.
// e.g.
/*
ddcoroutine<void> foo(DDCO_REF const std::string& p)
{
    co_await std::suspend_always();
    std::string p1 = p;
}
ddcoroutine<void> test_any()
{
    std::string p = "p";
    co_await foo(p); // Good, the p will not be release before foo finish.
    co_await foo("p"); // Not Good, temporary object will be release before foo finish.
    std::vector<ddcoroutine<void>> r;
    {
        std::string p = "p";
        r.push_back(foo(p)); // Not Good. C26811, the p will reference to a released object. the caller must ensure that the parameter is not released before the coroutine function finishes.
    }
    co_await ddcoroutine_all(r);
}
// In this `Not good` situation, the caller should create a wrapper function for it.
ddcoroutine<void> foo_safe(std::string p)
{
    co_return co_await foo(p);
}
*/
#define DDCO_REF

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
        : m_resumer(resumer) { }

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
    // use shared_ptr to make it copyabled
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

// ddco_task 的回调函数必须是异步的(在另外的线程调用, 或者在同一个线程的下一次loop中调用), 否则会引发栈溢出
// 如果用这样的需求, 使用ddawaitable_ex代替
inline auto ddawaitable(const ddco_task& task)
{
    struct ddawaiter
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_resume() const noexcept {}
        void await_suspend(std::coroutine_handle<> caller_handle) noexcept
        {
            if (m_task == nullptr) {
                return;
            }

            m_task(ddresume_helper([caller_handle, this]() {
                caller_handle.resume();
            }));
        }
        ddco_task m_task = nullptr;
    };
    return ddawaiter{ task };
}

// 允许类似:
// void foo(const std::function<void()>& callback)
// {
//     callback();
// }
// 这样的, 回调函数非异步的情况.
inline auto ddawaitable_ex(const ddco_task& task)
{
    struct ddawaiter
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_resume() const noexcept {}
        bool await_suspend(std::coroutine_handle<> caller_handle) noexcept
        {
            if (m_task == nullptr) {
                return false;
            }

            m_task(ddresume_helper([caller_handle, this]() {
                if (m_flag.exchange(true, std::memory_order_acq_rel)) {
                    // path 1
                    caller_handle.resume();
                } else {
                    // path 2
                    // 有以下两种情况, 都可以认为也是同步的, 这两种情况只要让await_suspend返回false来唤醒caller即可.
                    // 1. 该callback是同步的.
                    // 2. 线程调度的非常快, await_suspend还没返回就已经完成了
                }
            }));

            if (m_flag.exchange(true, std::memory_order_acq_rel)) {
                // path 2
                return false;
            } else {
                // path 1
                // !!! 非常危险 !!!, 请注意!!!
                // 运行到这里的时候, caller_handle.resume(); 可能已经在另外的一个线程中运行了
                // 这种情况下ddawaiter已经被析构了, 在使用任何该类的成员都是未定义行为, 包括返回std::coroutine_handle<>
                // 所以本函数选择返回bool类型而不是std::coroutine_handle<>类型
                return true;
            }
        }
        ddco_task m_task = nullptr;
        std::atomic_bool m_flag = false;
    };
    return ddawaiter{ task };
};

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
                // `ddcoroutine_noop::final_suspend` return `std::suspend_never`, so it will auto destroy.
                // and do not need r.destory() here.
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

inline void ddcoroutine_detach(const ddcoroutine<void>& r)
{
    ddcoroutine_run(r);
}

// do not use const XXX& r, because the ref of XXX may be released
// when this coroutine resume.
// e.g.:
// ddcoroutine<void> co_async() { co_return; }
// 
// ddcoroutine<void> test()
// {
//     auto x = ddcoroutine_from(co_async());
//     // if use const XXX& r, the ddcoroutine which is returned by co_async() had released, it is an UB.
//     co_await x;
// }
inline ddcoroutine<void> ddcoroutine_from(ddco_task task)
{
    co_await ddawaitable_ex(task);
}

template<class T>
inline ddcoroutine<void> ddcoroutine_from(ddcoroutine<T> r)
{
    co_await r;
}

inline ddcoroutine<void> ddcoroutine_from(std::vector<ddcoroutine<void>> r)
{
    for (const auto& it : r) {
        co_await it;
    }
}

inline ddcoroutine<void> ddcoroutine_all(const std::vector<ddcoroutine<void>>& r)
{
    return ddcoroutine_from(r);
}

// return the index of the first resumed ddcoroutine.
// return -1 when the r is empty;
// if detach_all is true, the other ddcoroutine will be detached when the first ddcoroutine is resumed.
// if detach_all is false, the other ddcoroutine still can be co_await.
// if you do not care about the other ddcoroutine when the first ddcoroutine is resumed, set detach_all to true.
inline ddcoroutine<int> ddcoroutine_any(std::vector<ddcoroutine<void>> r, bool detach_all = true)
{
    if (r.empty()) {
        co_return -1;
    }

    struct ddcoroutine_any_ctx
    {
        void resume(int index)
        {
            if (has_resumed_flag.exchange(true)) {
                return;
            }

            for (auto& it : coroutines) {
                auto awaiter = it.operator co_await();
                unsigned char expect = detail::ddflag_set_point::set_in_ddcoroutine_any_function;
                if (awaiter.m_self_ctx->set_point_flag.compare_exchange_strong(expect, (unsigned char)detail::ddflag_set_point::not_set, std::memory_order_acq_rel)) {
                    detail::ddcoroutine_any_resumer* resumer = (detail::ddcoroutine_any_resumer*)(awaiter.m_self_ctx->caller_co_handle);
                    awaiter.m_self_ctx->set_caller_co_handle(nullptr);
                    delete resumer;
                }
            }

            result_index = index;
            if (!flag.exchange(false)) {
                // resume
                if (caller_handle) {
                    caller_handle.resume();
                }
            }
        }
        std::atomic_bool flag = true;
        std::atomic_bool has_resumed_flag = false;
        std::vector<ddcoroutine<void>> coroutines;
        std::coroutine_handle<> caller_handle;
        int result_index = -1;
    };
    std::shared_ptr<ddcoroutine_any_ctx> any_ctx = std::make_shared<ddcoroutine_any_ctx>();
    any_ctx->coroutines = r;

    class ddcoroutine_any_resumer_impl : public detail::ddcoroutine_any_resumer
    {
    public:
        ddcoroutine_any_resumer_impl(const std::shared_ptr<ddcoroutine_any_ctx>& ctx, int index) :
            m_any_ctx(ctx), m_index(index) { }
        ~ddcoroutine_any_resumer_impl() override { }

        void resume() override
        {
            DDASSERT(m_any_ctx != nullptr);
            m_any_ctx->resume(m_index);
        }

        std::shared_ptr<ddcoroutine_any_ctx> m_any_ctx;
        int m_index = 0;
    };

    struct ddawaiter
    {
        constexpr bool await_ready() const noexcept { return false; }
        int await_resume() const noexcept { return any_ctx->result_index; }
        bool await_suspend(std::coroutine_handle<> caller_handle) noexcept
        {
            DDASSERT(any_ctx != nullptr);
            any_ctx->caller_handle = caller_handle;
            for (size_t i = 0; i < any_ctx->coroutines.size(); ++i) {
                auto& it = any_ctx->coroutines[i];
                auto awaiter = it.operator co_await();
                detail::ddcoroutine_any_resumer* resumer = new ddcoroutine_any_resumer_impl(any_ctx, (int)i);
                awaiter.m_self_ctx->set_caller_co_handle(resumer);
                if (!awaiter.m_self_ctx->check_unset_and_set(detail::ddflag_set_point::set_in_ddcoroutine_any_function)) {
                    resumer->resume();
                    delete resumer;
                    awaiter.m_self_ctx->set_caller_co_handle(nullptr);
                    break;
                }
            }

            return any_ctx->flag.exchange(false);
        }

        std::shared_ptr<ddcoroutine_any_ctx> any_ctx;
    };

    int index = co_await ddawaiter{ any_ctx };
    if (detach_all) {
        for (size_t i = 0; i < r.size(); ++i) {
            if (i != (size_t)index) {
                ddcoroutine_detach(r[i]);
            }
        }
    }
    co_return index;
}
} // namespace NSP_DD
#endif // c20
#endif // ddbase_coroutine_ddcoroutine_h_

