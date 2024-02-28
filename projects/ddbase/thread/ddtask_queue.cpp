#include "ddbase/stdafx.h"
#include "ddbase/thread/ddtask_queue.h"
#include "ddbase/ddtimer.h"
#include "ddbase/ddmath.h"
#include "ddbase/ddassert.h"
#include <atomic>

namespace NSP_DD {
/////////////////////ddtask/////////////////////
inline static u64 get_next_task_id()
{
    static std::atomic<u64> s_task_id = 0;
    return (++s_task_id);
}

ddtask::ddtask()
{
    m_id = get_next_task_id();
}

u64 ddtask::task_id()
{
    return m_id;
}

void ddtask::set_task_id(u64 id)
{
    m_id = id;
}

/////////////////////ddfunction_task/////////////////////
class ddfunction_task : public ddtask
{
public:
    ddfunction_task(const ddclosure& callback) : m_callback(callback)
    {
        DDASSERT(m_callback != nullptr);
    }
    void run_task() override
    {
        DDASSERT(m_callback != nullptr);
        m_callback();
    }
private:
    ddclosure m_callback;
};

/////////////////////ddtimeout_task/////////////////////
class ddtimeout_task : public ddfunction_task
{
public:
    ddtimeout_task(const ddclosure& callback, u64 timeout, u64 repeat_times)
        : ddfunction_task(callback) , m_timeout(timeout), m_repeat_times(repeat_times)
    {
        set_pre_time(ddtime::now_ms());
    }

    void set_pre_time(u64 pre_time)
    {
        m_pre_time = pre_time;
    }

    // 返回超时时间
    u64 get_idle_time(u64 now)
    {
        u64 time_pass = now - m_pre_time;
        if (m_timeout < time_pass || now < m_pre_time) {
            return 0;
        }

        return m_timeout - time_pass;
    }

    u64 consume_repeat_times()
    {
        if (m_repeat_times == 0) {
            return 0;
        }
        return --m_repeat_times;
    }

private:
    //上次执行时间(ms)，如果一次都没有执行过为创建时间
    u64 m_pre_time = 0;

    // 单位ms
    u64 m_timeout = 0;

    // 剩余执行次数
    u64 m_repeat_times = 0;
};

/////////////////////ddtask_queue/////////////////////
template <class T>
static inline bool wait_for_cv(ddcv& cv, T& mutex, u64 timeout)
{
    return cv.wait_for(mutex, timeout);
}

ddtask_queue::ddtask_queue(u64 max_cnt /* = MAX_U64 */)
    : m_max_count(max_cnt)
{
}

ddtask_queue::~ddtask_queue()
{
    stop_wait();
}

void ddtask_queue::set_max_count(u64 max_count)
{
    m_max_count = max_count;
}

u64 ddtask_queue::task_count()
{
    std::lock_guard<ddmutex> lock(m_mutex);
    return m_task_count;
}

void ddtask_queue::lock()
{
    m_mutex.lock();
}

void ddtask_queue::unlock()
{
    m_mutex.unlock();
}

bool ddtask_queue::wait_for_task(u64 timeout)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    u64 now = ddtime::now_ms();
    if (process_timeout_task(now)) {
        return true;
    }

    if (!m_ready_tasks.empty()) {
        return true;
    }

    if (m_next_timeout_task_ready_time != MAX_U64 && m_next_timeout_task_ready_time - now < timeout) {
        timeout = m_next_timeout_task_ready_time - now;
    }
    
    return wait_for_cv(m_wait_task_cv, m_mutex, timeout);
}

void ddtask_queue::stop_wait()
{
    m_wait_task_cv.notify_all();
}

sptask ddtask_queue::get_next_ready_task()
{
    std::lock_guard<ddmutex> lock(m_mutex);
    if (m_ready_tasks.empty()) {
        return nullptr;
    }
    auto it = m_ready_tasks.begin();
    sptask task = *it;
    (void)erase_from_ready_tasks(it);
    return task;
}

void ddtask_queue::run_all_ready_tasks()
{
    {
        std::lock_guard<ddmutex> lock(m_mutex);
        for (auto it = m_ready_tasks.begin(); it != m_ready_tasks.end();) {
            m_running_tasks.push_back(*it);
            it = erase_from_ready_tasks(it);
        }
    }

    while (true) {
        m_mutex.lock();
        if (m_running_tasks.empty()) {
            m_mutex.unlock();
            return;
        }
        auto it = m_running_tasks.begin();
        sptask task = *it;
        (void)m_running_tasks.erase(it);
        m_mutex.unlock();
        (void)task->run_task();
    }
}

u64 ddtask_queue::push_task(const sptask& task, u64 push_timeout /*= MAX_U64*/)
{
    if (task == nullptr || !wait_max_count_cv(push_timeout)) {
        return 0;
    }

    std::lock_guard<ddmutex> lock(m_mutex);
    ++m_task_count;
    m_ready_tasks.push_back(task);
    m_wait_task_cv.notify_one();
    return task->task_id();
}

u64 ddtask_queue::push_task(const ddclosure& task, u64 push_timeout /*= MAX_U64*/)
{
    if (task == nullptr) {
        return 0;
    }
    return push_task(sptask(new(std::nothrow) ddfunction_task(task)), push_timeout);
}

u64 ddtask_queue::push_timeout_task_inner(const sptask& task, u64 push_timeout)
{
    if (task == nullptr || !wait_max_count_cv(push_timeout)) {
        return 0;
    }

    std::lock_guard<ddmutex> lock(m_mutex);
    ++m_task_count;
    m_timeout_tasks.push_back(task);
    m_next_timeout_task_ready_time = 0;
    m_wait_task_cv.notify_one();
    return task->task_id();
}

u64 ddtask_queue::push_task(const sptask& task, u64 task_timeout, u64 times, u64 push_timeout)
{
    sptask timeout_task(new(std::nothrow)ddtimeout_task([task]() {
        return task->run_task();
    }, task_timeout, times));
    if (task == nullptr) {
        return 0;
    }
    timeout_task->set_task_id(task->task_id());
    return push_timeout_task_inner(timeout_task, push_timeout);
}

u64 ddtask_queue::push_task(const ddclosure& task, u64 task_timeout, u64 times, u64 push_timeout)
{
    return push_timeout_task_inner(sptask(new(std::nothrow)ddtimeout_task(task, task_timeout, times)), push_timeout);
}

bool ddtask_queue::process_timeout_task(u64 now)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    if (m_timeout_tasks.empty()) {
        m_next_timeout_task_ready_time = MAX_U64;
        return false;
    }
    if (m_next_timeout_task_ready_time > now) {
        return false;
    }
    u64 min_timeout = MAX_U64;
    bool result = false;
    for (auto it = m_timeout_tasks.begin(); it != m_timeout_tasks.end();) {
        ddtimeout_task* task = (ddtimeout_task*)((*it).get());
        u64 idle_time = task->get_idle_time(now);
        if (idle_time == 0) {
            if (m_ready_timeout_tasks.find(task->task_id()) == m_ready_timeout_tasks.end()) {
                result = true;
                m_ready_timeout_tasks.insert(task->task_id());
                m_ready_tasks.push_back(*it);
                if (task->consume_repeat_times() == 0) {
                    it = m_timeout_tasks.erase(it);
                    --m_task_count;
                    m_max_count_cv.notify_one();
                    continue;
                }
            }

            task->set_pre_time(now);
            idle_time = task->get_idle_time(now);
        }

        if (min_timeout > idle_time) {
            min_timeout = idle_time;
        }
        ++it;
    }

    if (min_timeout == MAX_U64) {
        m_next_timeout_task_ready_time = MAX_U64;
    } else {
        m_next_timeout_task_ready_time = now + min_timeout;
    }
    return result;
}

std::list<sptask>::iterator ddtask_queue::erase_from_ready_tasks(const std::list<sptask>::iterator& it)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    if (m_ready_timeout_tasks.find((*it)->task_id()) != m_ready_timeout_tasks.end()) {
        m_ready_timeout_tasks.erase((*it)->task_id());
        ((ddtimeout_task*)((*it).get()))->set_pre_time(ddtime::now_ms());
    } else {
        --m_task_count;
        m_max_count_cv.notify_one();
    }
    return m_ready_tasks.erase(it);
}

bool ddtask_queue::die_task(u64 task_id)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    bool find = false;
    for (auto it = m_running_tasks.begin(); it != m_running_tasks.end();) {
        if (task_id == (*it)->task_id()) {
            it = m_running_tasks.erase(it);
            find = true;
        }
    }

    for (auto it = m_ready_tasks.begin(); it != m_ready_tasks.end();) {
        if (task_id == (*it)->task_id()) {
            it = erase_from_ready_tasks(it);
            find = true;
        }
    }

    for (auto it = m_timeout_tasks.begin(); it != m_timeout_tasks.end();) {
        if (task_id == (*it)->task_id()) {
            it = m_timeout_tasks.erase(it);
            --m_task_count;
            m_max_count_cv.notify_one();
            find = true;
        }
    }
    return find;
}

void ddtask_queue::die_all_tasks()
{
    std::lock_guard<ddmutex> lock(m_mutex);
    for (u64 i = 0; i < task_count(); ++i) {
        m_max_count_cv.notify_one();
    }
    m_ready_tasks.clear();
    m_timeout_tasks.clear();
    m_ready_timeout_tasks.clear();
    m_running_tasks.clear();
    m_task_count = 0;
}

bool ddtask_queue::wait_max_count_cv(u64 timeout)
{
    if (task_count() < m_max_count) {
        return true;
    }

    return wait_for_cv(m_max_count_cv, m_mutex, timeout);
}
} // namespace NSP_DD
