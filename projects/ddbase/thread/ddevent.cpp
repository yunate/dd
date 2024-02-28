#include "ddbase/stdafx.h"
#include "ddbase/thread/ddevent.h"
#include "ddbase/ddtimer.h"

namespace NSP_DD {

struct u32_is_0 {
    const u32& m_u32;
    bool operator()() const
    {
        return m_u32 == 0;
    }
};

////////////////////////////////////ddmutex////////////////////////////////////
void ddmutex::lock()
{
    const std::thread::id thread_id = std::this_thread::get_id();
    std::unique_lock<std::mutex> lock(m_mutex);
    if (thread_id == m_thread_id) {
        if (m_locked_count < MAX_U32) {
            ++m_locked_count;
        }
    } else {
        while (m_locked_count != 0) {
            m_cv.wait(lock);
        }
        m_locked_count = 1;
        m_thread_id = thread_id;
    }
}

void ddmutex::unlock()
{
    bool notify = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        --m_locked_count;
        if (m_locked_count == 0) {
            m_thread_id = std::thread::id();
            notify = true;
        }
    }

    if (notify) {
        m_cv.notify_one();
    }
}

bool ddmutex::try_lock()
{
    const std::thread::id thread_id = std::this_thread::get_id();
    std::lock_guard<std::mutex> lock(m_mutex);

    if (thread_id == m_thread_id) {
        if (m_locked_count < MAX_U32) {
            ++m_locked_count;
        } else {
            return false;
        }
    } else {
        if (m_locked_count != 0) {
            return false;
        } else {
            m_locked_count = 1;
            m_thread_id = thread_id;
        }
    }
    return true;
}

bool ddmutex::try_lock_for(u64 ms)
{
    const std::thread::id thread_id = std::this_thread::get_id();
    std::unique_lock<std::mutex> lock(m_mutex);

    if (thread_id == m_thread_id) {
        if (m_locked_count < MAX_U32) {
            ++m_locked_count;
        } else {
            return false;
        }
    } else {
        if (!m_cv.wait_for(lock, std::chrono::duration<u64, std::milli>(ms), u32_is_0{ m_locked_count })) {
            return false;
        }
        m_locked_count = 1;
        m_thread_id = thread_id;
    }
    return true;
}

////////////////////////////////////ddcv////////////////////////////////////
void ddcv::notify_one()
{
    m_cv.notify_one();
}

void ddcv::notify_all()
{
    m_cv.notify_all();
}

void ddcv::wait(ddmutex& mutex)
{
    std::lock_guard<ddmutex> lock(mutex);
    u32 count = mutex.m_locked_count;
    mutex.m_locked_count = 1;
    m_cv.wait(mutex);
    mutex.m_locked_count = count;
}

bool ddcv::wait_for(ddmutex& mutex, u64 ms)
{
    bool timeout = false;
    std::lock_guard<ddmutex> lock(mutex);
    u32 count = mutex.m_locked_count;
    mutex.m_locked_count = 1;
    if (m_cv.wait_for(mutex, std::chrono::duration<u64, std::milli>(ms))
        == std::cv_status::timeout) {
        timeout = true;
    }
    mutex.m_locked_count = count;
    return timeout;
}

////////////////////////////////////ddevent////////////////////////////////////
bool ddevent::wait(u64 timeout /*= MAX_U64*/)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    bool is_timeout = false;
    while (!m_hasSignal && !m_notyfiall) {
        if (m_con.wait_for(lck, std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
            is_timeout = true;
            break;
        }
    }

    if (!is_timeout && !m_notyfiall) {
        m_hasSignal = false;
    }
    return !is_timeout;
}

void ddevent::notify_one()
{
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_hasSignal = true;
    }
    m_con.notify_one();
}

void ddevent::notify_all()
{
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_notyfiall = true;
        m_hasSignal = true;
    }
    m_con.notify_all();
}

void ddevent::reset(bool signal)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_notyfiall = false;
    m_hasSignal = signal;
}
////////////////////////////////////ddsemaphore////////////////////////////////////
bool ddsemaphore::wait(u64 timeout /*= MAX_U64*/)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    bool is_timeout = false;
    while (m_count <= 0 && !m_notyfiall) {
        if (m_con.wait_for(lck, std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
            is_timeout = true;
            break;
        }
    }

    if (!is_timeout && !m_notyfiall) {
        --m_count;
    }

    if (m_count < 0) {
        m_count = 0;
    }
    return !timeout;
}

void ddsemaphore::notify_one()
{
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        ++m_count;
    }
    m_con.notify_one();
}

void ddsemaphore::notify_all()
{
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_notyfiall = true;
        ++m_count;
    }
    m_con.notify_all();
}

void ddsemaphore::reset(u32 cnt)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_notyfiall = false;
    m_count = cnt;
}

////////////////////////////////////spin_mutex////////////////////////////////////
// #define USE_ASM_CPUPAUSE
#ifdef USE_ASM_CPUPAUSE
#ifdef _WIN64 
#define cpu_pause ddcpu_pause_asm();
#else
#define cpu_pause __asm{pause}
#endif
#else
#include <windows.h>
#define cpu_pause ;
#endif

void ddspin_mutex::lock()
{
    // test_and_set 返回m_falg的设置前的值
    while (m_flag.test_and_set() != false) {
        cpu_pause;
    }
}
void ddspin_mutex::unlock()
{
    m_flag.clear();
}
} // namespace NSP_DD
