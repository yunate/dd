
#ifndef ddbase_thread_ddevent_h_
#define ddbase_thread_ddevent_h_

#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"
#include "ddbase/ddnocopyable.hpp"
#include <mutex>
#include <condition_variable>

namespace NSP_DD {
class ddcv
{
    DDNO_COPY_MOVE(ddcv);
public:
    ddcv() = default;
    ~ddcv() = default;

    // wait causes the current thread to block until the ddcv is notified.
    // @note1 it maybe wakeup when the ddcv is not notified, this is so called spurious wakeup occurs.
    // @note2 if the ddcv is notified before wait, it will not be wakeup.
    // set pred to check yourself condition to avoid encountering either of these situations
    // if wait_time equal to MAX_U64, it means never timeout;
    // return if time out;
    bool wait(std::unique_lock<std::mutex>& lock, u64 wait_time = MAX_U64, const std::function<bool()>& pred = nullptr);

    void notify_one();
    void notify_all();
private:
    bool wait_internal(std::unique_lock<std::mutex>& lock, u64 wait_time);
    std::condition_variable m_cv;
};

class ddevent
{
    DDNO_COPY_MOVE(ddevent);
public:
    ddevent() = default;
    ~ddevent() = default;
    // wait causes the current thread to block until notify.
    // if wait_time equal to MAX_U64, it means never timeout;
    // consume_all means consume all the notify, otherwise consume one notify.
    bool wait(u64 wait_time = MAX_U64, bool consume_all = true);
    void notify(u32 count = 1);
    void reset();

private:
    std::mutex m_mutex;
    ddcv m_cv;
    u64 m_ready_count = 0;
};
} // namespace NSP_DD
#endif // ddbase_thread_ddevent_h_

