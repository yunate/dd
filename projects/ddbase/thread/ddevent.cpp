#include "ddbase/stdafx.h"
#include "ddbase/thread/ddevent.h"
#include "ddbase/ddtime.h"
#include <chrono>
namespace NSP_DD {
bool ddcv::wait_internal(std::unique_lock<std::mutex>& lock, u64 wait_time)
{
    if (wait_time == MAX_U64) {
        m_cv.wait(lock);
    } else {
        if (m_cv.wait_for(lock, std::chrono::milliseconds(wait_time)) == std::cv_status::timeout) {
            return false;
        }
    }
    return true;
}

bool ddcv::wait(std::unique_lock<std::mutex>& lock, u64 wait_time /* = MAX_U64 */, const std::function<bool()>& pred /*  = nullptr */)
{
    if (pred == nullptr) {
        return wait_internal(lock, wait_time);
    }

    while (!pred()) {
        if (!wait(lock, wait_time)) {
            return pred();
        }
    }
    
    return true;
}

void ddcv::notify_one()
{
    m_cv.notify_one();
}

void ddcv::notify_all()
{
    m_cv.notify_all();
}

//////////////////////////////////////ddevent//////////////////////////////////////
bool ddevent::wait(u64 wait_time /* = MAX_U64 */, bool consume_all /* = true */)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_cv.wait(lock, wait_time, [this, consume_all]() {
        if (m_ready_count > 0) {
            if (consume_all) {
                m_ready_count = 0;
            } else {
                --m_ready_count;
            }
            return true;
        }

        return false;
    });
}

void ddevent::notify(u32 count /* = 1 */)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_ready_count += count;
    m_cv.notify_all();
}

void ddevent::reset()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_ready_count = 0;
}
} // namespace NSP_DD
