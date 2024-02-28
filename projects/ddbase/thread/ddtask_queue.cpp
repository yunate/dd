#include "ddbase/stdafx.h"
#include "ddbase/thread/ddtask_queue.h"
#include "ddbase/ddtime.h"
#include "ddbase/ddmath.h"
#include "ddbase/ddassert.h"
#include <atomic>

namespace NSP_DD {
inline static u64 get_next_task_id()
{
    static std::atomic<u64> s_task_id = 0;
    return (++s_task_id);
}

class ddtimeout_task : public ddtask
{
public:
    ddtimeout_task(const std::function<void()>& callback, u64 timeout, u64 repeat_times)
        : m_callback(callback), m_timeout(timeout), m_repeat_times(repeat_times)
    {
        DDASSERT(m_callback != nullptr);
        m_id = get_next_task_id();
        m_last_run_time = ddtime::now_ms();
    }

    void run_task() override
    {
        if (m_repeat_times == 0) {
            return;
        }

        if (m_repeat_times != MAX_U64) {
            --m_repeat_times;
        }
        m_last_run_time = ddtime::now_ms();
        DDASSERT(m_callback != nullptr);
        m_callback();
    }

    u64 get_task_id() const
    {
        return m_id;
    }

    void set_task_id(u64 id)
    {
        m_id = id;
    }

    // 返回超时时间
    u64 get_idle_time(u64 now) const
    {
        u64 time_pass = now - m_last_run_time;
        if (m_timeout < time_pass || now < m_last_run_time) {
            return 0;
        }

        return m_timeout - time_pass;
    }

    u64 get_repeat_times() const
    {
        return m_repeat_times;
    }

    std::atomic_uint REF_COUNT = 1;

private:
    u64 m_id = 0;
    std::function<void()> m_callback;

    //上次执行时间(ms)，如果一次都没有执行过为创建时间
    u64 m_last_run_time = 0;

    // 单位ms
    u64 m_timeout = 0;

    // 剩余执行次数
    u64 m_repeat_times = 0;
};

static inline void decre_task_ref(ddtask* task)
{
    ddtimeout_task* timeout_task = (ddtimeout_task*)task;
    if (timeout_task->REF_COUNT.fetch_sub(1) == 1) {
        delete timeout_task;
    }
}

static inline void incre_task_ref(ddtask* task)
{
    ddtimeout_task* timeout_task = (ddtimeout_task*)task;
    timeout_task->REF_COUNT.fetch_add(1);
}

/////////////////////ddtask_queue/////////////////////
ddtask_queue::ddtask_queue(u32 max_count /* = MAX_S32 */, const std::function<void(u32 count)>& on_any_task_complete /* = nullptr */) :
    m_max_count(max_count), m_on_any_task_complete(on_any_task_complete)
{ }

ddtask_queue::~ddtask_queue()
{
    kill_all_tasks();
    notify();
}

void ddtask_queue::set_max_count(u32 max_count)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_max_count = max_count;
}

void ddtask_queue::set_any_task_complete_handler(const std::function<void(u32 count)>& on_any_task_complete)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_on_any_task_complete = on_any_task_complete;
}

u32 ddtask_queue::get_max_count()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_max_count;
}

u64 ddtask_queue::get_task_count()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return (u64)m_tasks.size();
}

bool ddtask_queue::run_and_wait(u64 timeout /* = MAX_U64 */)
{
#ifdef _DEBUG
    {
        DDASSERT_FMTW(!m_run_and_waiting, L"run_and_wait can run in only one thread!!!");
        m_run_and_waiting = true;
    }

    ddexec_guard guard([this]() {
        m_run_and_waiting = false;
    });
#endif

    s64 min_idle_time = timeout;
    u64 now = ddtime::now_ms();
    std::list<ddtask*> need_delete_tasks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_tasks.empty()) {
            auto it = m_tasks.begin();
            for (; it != m_tasks.end();) {
                ddtimeout_task* task = (ddtimeout_task*)(*it);
                if (task->get_repeat_times() == 0) {
                    need_delete_tasks.push_back(*it);
                    it = m_tasks.erase(it);
                    continue;
                }

                s64 idle_time = (s64)task->get_idle_time(now);
                if (idle_time == 0) {
                    m_ready_tasks.push_back(*it);
                } else if (idle_time < min_idle_time) {
                    min_idle_time = idle_time;
                }
                ++it;
            }
        }
    }

    while (true) {
        ddtimeout_task* task = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_ready_tasks.empty()) {
                task = (ddtimeout_task*)m_ready_tasks.front();
                m_ready_tasks.pop_front();
                incre_task_ref(task);
            }
        }
        if (task == nullptr) {
            break;
        }
        task->run_task();
        decre_task_ref(task);
    }

    if (!need_delete_tasks.empty()) {
        for (auto it : need_delete_tasks) {
            decre_task_ref(it);
        }
        if (m_on_any_task_complete != nullptr) {
            m_on_any_task_complete((u32)need_delete_tasks.size());
        }
    }

    min_idle_time -= (ddtime::now_ms() - now);
    if (min_idle_time <= 0) {
        return true;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    return m_cv.wait(lock, min_idle_time, [this]() {
        if (m_noifyed || !m_tasks.empty()) {
            m_noifyed = false;
            return true;
        }

        return false;
    });
}

void ddtask_queue::notify()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_noifyed = true;
    m_cv.notify_one();
}

u64 ddtask_queue::push_task(const std::function<void()>& task, u64 task_timeout /* = 0 */, u64 run_times /* = 1 */)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (task == nullptr) {
        return 0;
    }

    if (m_max_count <= (u32)m_tasks.size()) {
        return 0;
    }

    ddtimeout_task* timeout_task = new (std::nothrow) ddtimeout_task(task, task_timeout, run_times);
    if (timeout_task == nullptr) {
        return 0;
    }

    m_tasks.push_back(timeout_task);
    m_noifyed = true;
    m_cv.notify_one();
    return timeout_task->get_task_id();
}

u64 ddtask_queue::push_task(const std::shared_ptr<ddtask>& task, u64 task_timeout /* = 0 */, u64 run_times /* = 1 */)
{
    return push_task([task]() {
        if (task != nullptr) {
            task->run_task();
        }
    }, task_timeout, run_times);
}

std::shared_ptr<ddevent> ddtask_queue::push_flag_task()
{
    class ddflag_task : public ddtask
    {
    public:
        ddflag_task()
        {
            m_event.reset(new(std::nothrow) ddevent());
        }
        virtual ~ddflag_task()
        {
            if (m_event) {
                m_event->notify();
            }
        }
        void run_task() override
        {
            if (m_event) {
                m_event->notify();
            }
        }

        std::shared_ptr<ddevent> m_event;
    };

    std::shared_ptr<ddflag_task> task(new(std::nothrow) ddflag_task());
    if (task == nullptr) {
        return nullptr;
    }
    auto return_event = task->m_event;
    if (return_event == nullptr) {
        return nullptr;
    }
    push_task(task);
    return return_event;
}

bool ddtask_queue::kill_task(u64 task_id)
{
    bool find = false;
    ddtimeout_task* need_delete_task = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_ready_tasks.begin(); it != m_ready_tasks.end(); ++it) {
            ddtimeout_task* task = (ddtimeout_task*)(*it);
            if (task->get_task_id() == task_id) {
                m_ready_tasks.erase(it);
                find = true;
            }
        }

        for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it) {
            ddtimeout_task* task = (ddtimeout_task*)(*it);
            if (task->get_task_id() == task_id) {
                need_delete_task = task;
                m_tasks.erase(it);
                find = true;
            }
        }
    }

    decre_task_ref(need_delete_task);
    return find;
}

void ddtask_queue::kill_all_tasks()
{
    std::list<ddtask*> need_delete_tasks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ready_tasks.clear();
        m_tasks.swap(need_delete_tasks);
    }

    for (auto it : need_delete_tasks) {
        decre_task_ref(it);
    }
}
} // namespace NSP_DD
