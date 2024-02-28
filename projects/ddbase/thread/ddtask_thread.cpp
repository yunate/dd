#include "ddbase/stdafx.h"
#include "ddbase/thread/ddtask_thread.h"
namespace NSP_DD {
ddtask_thread::~ddtask_thread()
{
    stop(0);
}

ddtask_queue& ddtask_thread::get_task_queue()
{
    return m_task_queue;
}

void ddtask_thread::join()
{
    if (m_thread != nullptr && m_thread->joinable()) {
        m_thread->join();
    }
}

bool ddtask_thread::stop(u64 waitTime)
{
    {
        std::lock_guard<ddtask_queue> lock(m_task_queue);
        m_stop = true;
        m_task_queue.die_all_tasks();
        m_task_queue.stop_wait();
    }
    bool result = true;
    if (m_loop != nullptr) {
        result = m_loop->stop(waitTime);
    }
    if (m_thread != nullptr) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    if (m_loop != nullptr) {
        delete m_loop;
        m_loop = nullptr;
    }
    return result;
}

bool ddtask_thread::start()
{
    if (m_thread != nullptr) {
        return true;
    }

    m_stop = false;
    m_loop = new (std::nothrow) ddfunction_loop([this]()
    {
        while (true) {
            {
                std::lock_guard<ddtask_queue> lock(m_task_queue);
                if (m_stop) {
                    break;
                }

                if (!m_task_queue.wait_for_task()) {
                    continue;
                }
            }
            (void)m_task_queue.run_all_ready_tasks();
        }
    });
    if (m_loop == nullptr) {
        return false;
    }
    m_thread = new(std::nothrow)std::thread(&ddfunction_loop::loop, m_loop);
    return m_thread != nullptr;
}

bool ddtask_thread::has_start()
{
    return (m_thread != nullptr);
}

//////////////////////////////ddtask_thread_pool//////////////////////////////

ddtask_thread_pool::ddtask_thread_pool(u32 thread_count)
    : m_max_count(thread_count)
{
}

ddtask_thread_pool::~ddtask_thread_pool()
{
    std::lock_guard<ddmutex> lock(m_mutex);
    stop_all();
}

void ddtask_thread_pool::stop_all()
{
    std::lock_guard<ddmutex> lock(m_mutex);
    m_task_threads.clear();
}

u64 ddtask_thread_pool::push_task(const sptask& task, u64 push_timeout)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    auto it = get_task_thread();
    if (it == nullptr) {
        return 0;
    }

    return it->get_task_queue().push_task(task, push_timeout);
}

u64 ddtask_thread_pool::push_task(const ddclosure& task, u64 push_timeout)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    auto it = get_task_thread();
    if (it == nullptr) {
        return 0;
    }

    return it->get_task_queue().push_task(task, push_timeout);
}

u64 ddtask_thread_pool::push_task(const sptask& task, u64 task_timeout, u64 times, u64 push_timeout)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    auto it = get_task_thread();
    if (it == nullptr) {
        return 0;
    }

    return it->get_task_queue().push_task(task, task_timeout, times, push_timeout);
}

u64 ddtask_thread_pool::push_task(const ddclosure& task, u64 task_timeout, u64 times, u64 push_timeout)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    auto it = get_task_thread();
    if (it == nullptr) {
        return 0;
    }

    return it->get_task_queue().push_task(task, task_timeout, times, push_timeout);
}

bool ddtask_thread_pool::die_task(u64 task_id)
{
    std::lock_guard<ddmutex> lock(m_mutex);
    for (auto& it : m_task_threads) {
        if (it->get_task_queue().die_task(task_id)) {
            return true;
        }
    }
    return false;
}

void ddtask_thread_pool::die_all_tasks()
{
    std::lock_guard<ddmutex> lock(m_mutex);
    for (auto& it : m_task_threads) {
        it->get_task_queue().die_all_tasks();
    }
}

std::shared_ptr<ddtask_thread> ddtask_thread_pool::get_task_thread()
{
    std::lock_guard<ddmutex> lock(m_mutex);
    std::shared_ptr<ddtask_thread> tmp;
    if (m_task_threads.empty()) {
        tmp.reset(new (std::nothrow) ddtask_thread());
        if (tmp != nullptr) {
            m_task_threads.push_back(tmp);
            tmp->start();
        }
    } else {
        tmp = m_task_threads[0];
        for (size_t i = 1; i < m_task_threads.size(); ++i) {
            if (tmp->get_task_queue().task_count() > m_task_threads[i]->get_task_queue().task_count()) {
                tmp = m_task_threads[i];
            }
        }

        if (tmp->get_task_queue().task_count() != 0 && m_task_threads.size() < m_max_count) {
            ddtask_thread* task_thread = new (std::nothrow) ddtask_thread();
            if (task_thread != nullptr) {
                tmp.reset(task_thread);
                m_task_threads.push_back(tmp);
                tmp->start();
            }
        }
    }
    return tmp;
}

} // namespace NSP_DD
