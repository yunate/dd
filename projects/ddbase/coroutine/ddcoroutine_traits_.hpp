#ifndef ddbase_coroutine_ddcoroutine_traits__hpp_
#define ddbase_coroutine_ddcoroutine_traits__hpp_
#include "ddbase/ddassert.h"
#include "ddbase/coroutine/ddcoroutine_context_.hpp"

#if _HAS_CXX20
#include <coroutine>
#include <memory>

namespace NSP_DD {
template<class T>
class ddcoroutine_traits
{
public:
    class ddcoroutine_base
    {
    public:
        const T& get_value()
        {
            DDASSERT(m_ctx != nullptr);
            return m_ctx->get_value();
        }
    protected:
        std::shared_ptr<ddcoroutine_context<T>> m_ctx = nullptr;
    };

    class ddpromise_type_base
    {
    public:
        void return_value(T&& v)
        {
            DDASSERT(m_self_ctx != nullptr);
            m_self_ctx->set_value(std::forward<T>(v));
        }

        void return_value(const T& v)
        {
            DDASSERT(m_self_ctx != nullptr);
            m_self_ctx->set_value(v);
        }

        ddcoroutine_context<T>* m_self_ctx = nullptr;
    };

    struct ddawaiter_base
    {
        const T& await_resume() const noexcept
        {
            // step 4. when the caller had been resumed in step 3, return the called value to the caller's co_await.
            if constexpr (!std::is_same_v<T, void>) {
                DDASSERT(m_self_ctx != nullptr);
                return m_self_ctx->get_value();
            }
        }
        ddcoroutine_context<T>* m_self_ctx = nullptr;
    };
};

template<>
class ddcoroutine_traits<void>
{
public:
    class ddcoroutine_base
    {
    protected:
        std::shared_ptr<ddcoroutine_context<void>> m_ctx = nullptr;
    };

    class ddpromise_type_base
    {
    public:
        void return_void() {}
        ddcoroutine_context<void>* m_self_ctx = nullptr;
    };

    struct ddawaiter_base
    {
        void await_resume() const noexcept
        {
            // step 4. when the caller had been resumed in step 3, return the called value to the caller's co_await.
        }
        ddcoroutine_context<void>* m_self_ctx = nullptr;
    };
};
} // namespace NSP_DD
#endif // c20
#endif // ddbase_coroutine_ddcoroutine_traits__hpp_
