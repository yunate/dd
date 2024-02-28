#include "ddbase/stdafx.h"
#include "ddbase/thread/ddloop.h"
#include "ddbase/ddassert.h"
namespace NSP_DD {

ddloop::~ddloop()
{
    // 不应该调用stop，因为loop 是由子类实现的，该类完全是以工具函数的方式提供了stop 和 wait_loop_end来减少代码重复
    // 另外stop 是虚函数，子类如果重写了该函数那么这儿调用会一定程度上造成误解
    // (void)stop(MAX_U32);
}

bool ddloop::stop(u64 timeout)
{
    if (m_mutex.try_lock_for(std::chrono::milliseconds(timeout))) {
        m_mutex.unlock();
        return true;
    }
    return false;
}

void ddloop::loop()
{
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    loop_core();
}

//////////////////////////////ddfunction_loop//////////////////////////////
ddfunction_loop::ddfunction_loop(const std::function<void()>& callback)
    : m_callback(callback)
{
}

void ddfunction_loop::loop_core()
{
    if (m_callback != nullptr) {
        m_callback();
    }
}

//////////////////////////////ddwin_msg_loop//////////////////////////////
bool ddwin_msg_loop::stop(u64)
{
    if (::PostThreadMessage(m_loopId, WM_QUIT, NULL, NULL)) {
        m_stop = true;
    }
    return m_stop;
}

void ddwin_msg_loop::loop()
{
    m_loopId = ::GetCurrentThreadId();
    while (!m_stop)
    {
        BOOL hr = ::GetMessage(&m_msg, NULL, 0, 0);
        if (hr == 0) {
            // WM_QUIT, exit message loop
            m_stop = true;
            continue;
        } else if (hr == -1) {
            continue;
        }

        if (!filter_msg(&m_msg))
        {
            ::TranslateMessage(&m_msg);
            ::DispatchMessage(&m_msg);
        }
    }
}

bool ddwin_msg_loop::filter_msg(MSG* msg)
{
    (msg);
    return false;
}

} // namespace NSP_DD
