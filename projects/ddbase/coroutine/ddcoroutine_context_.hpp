#ifndef ddbase_coroutine_ddcoroutine_context__hpp_
#define ddbase_coroutine_ddcoroutine_context__hpp_
#include <coroutine>
#if _HAS_CXX20
#include "ddbase/ddassert.h"

namespace NSP_DD {
template<class T>
class ddcoroutine_context_base
{
public:
    const T& get_value()
    {
        return m_value;
    }

    void set_value(T&& v)
    {
        m_value = std::forward<T>(v);
    }

    void set_value(const T& v)
    {
        m_value = v;
    }
protected:
    T m_value{};
};

template<>
class ddcoroutine_context_base<void> { };

template<class T>
class ddcoroutine_context : public ddcoroutine_context_base<T>
{
public:
    ~ddcoroutine_context()
    {
        if (m_self_co_handle) {
            std::coroutine_handle<> tmp = m_self_co_handle;
            reset();
            tmp.destroy();
        }
    }

    inline void set_self_co_handle(std::coroutine_handle<> co_handle)
    {
        m_self_co_handle = co_handle;
    }

    inline std::coroutine_handle<> get_self_co_handle()
    {
        return m_self_co_handle;
    }

    inline void resume()
    {
        DDASSERT(m_self_co_handle);
        m_self_co_handle.resume();
    }

    inline void set_caller_co_handle(std::coroutine_handle<> co_handle)
    {
        m_caller_co_handle = co_handle;
    }

    inline std::coroutine_handle<> get_caller_co_handle()
    {
        return m_caller_co_handle;
    }

    inline void reset()
    {
        m_self_co_handle = nullptr;
        m_caller_co_handle = nullptr;
    }

private:
    std::coroutine_handle<> m_self_co_handle = nullptr;
    std::coroutine_handle<> m_caller_co_handle = nullptr;
};

} // namespace NSP_DD
#endif // c20
#endif // ddbase_coroutine_ddcoroutine_context__hpp_
