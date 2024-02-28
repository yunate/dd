#include "ddbase/stdafx.h"
#include "ddbase/thread/ddtask_thread.h"
namespace NSP_DD {
ddtask_thread::~ddtask_thread()
{
    stop();
}

ddtask_queue& ddtask_thread::get_task_queue()
{
    return m_task_queue;
}

bool ddtask_thread::stop()
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        // Push an empty task to notify the wait.
        m_task_queue.push_task([]() {});
        m_task_queue.kill_all_tasks();
        m_stop = true;
    }

    if (m_thread != nullptr) {
        if (m_thread->joinable()) {
            m_thread->join();
        }

        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        delete m_thread;
        m_thread = nullptr;
    }
    return true;
}

bool ddtask_thread::start()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_thread != nullptr) {
        return true;
    }

    m_stop = false;
    m_thread = new(std::nothrow)std::thread([this]() {
        while (true) {
            {
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                if (m_stop) {
                    break;
                }
            }

            (void)m_task_queue.run_and_wait(1000);
        }
    });
    return m_thread != nullptr;
}

//////////////////////////////ddtask_thread_pool//////////////////////////////
ddtask_thread_pool::ddtask_thread_pool(u32 thread_count, u32 pre_thread_task_count /* = MAX_U32 */)
    : m_max_thread_count(thread_count), m_pre_thread_task_count(pre_thread_task_count)
{
}

ddtask_thread_pool::~ddtask_thread_pool()
{
    stop_all();
}

void ddtask_thread_pool::stop_all()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_task_threads.clear();
}

bool ddtask_thread_pool::kill_task(u64 task_id)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (auto& it : m_task_threads) {
        if (it->get_task_queue().kill_task(task_id)) {
            return true;
        }
    }
    return false;
}

void ddtask_thread_pool::kill_all_tasks()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (auto& it : m_task_threads) {
        it->get_task_queue().kill_all_tasks();
    }
}

u64 ddtask_thread_pool::push_task(const std::function<void()>& task, u64 task_timeout, u64 run_times)
{
    std::shared_ptr<ddtask_thread> task_thread = get_task_thread();
    if (task_thread == nullptr) {
        return 0;
    }

    return task_thread->get_task_queue().push_task(task, task_timeout, run_times);
}

u64 ddtask_thread_pool::push_task(const std::shared_ptr<ddtask>& task, u64 task_timeout, u64 run_times)
{
    std::shared_ptr<ddtask_thread> task_thread = get_task_thread();
    if (task_thread == nullptr) {
        return 0;
    }

    return task_thread->get_task_queue().push_task(task, task_timeout, run_times);
}

bool ddtask_thread_pool::wait_ready_to_push(u64 timeout /* = MAX_U64 */)
{
    return m_event.wait(timeout);
}

bool ddtask_thread_pool::wait_end(u64 timeout /* = MAX_U64 */)
{
    ddexpire expire = ddexpire::form_timeout((u32)timeout);
    for (auto& it : m_task_threads) {
        auto event = it->get_task_queue().push_flag_task();
        if (event != nullptr) {
            if (!event->wait(expire.get_timeout())) {
                return false;
            }
        }
    }

    return true;
}

std::shared_ptr<ddtask_thread> ddtask_thread_pool::create_task_thread()
{
    std::shared_ptr<ddtask_thread> tmp(new (std::nothrow) ddtask_thread());
    if (tmp != nullptr) {
        tmp->get_task_queue().set_max_count(m_pre_thread_task_count);
        tmp->get_task_queue().set_any_task_complete_handler([this](u32 count) {
            m_event.notify(count);
        });
        m_task_threads.push_back(tmp);
        tmp->start();
    }
    return tmp;
}

std::shared_ptr<ddtask_thread> ddtask_thread_pool::get_task_thread()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    std::shared_ptr<ddtask_thread> tmp;
    if (m_task_threads.empty()) {
        tmp = create_task_thread();
    } else {
        tmp = m_task_threads[0];
        for (size_t i = 1; i < m_task_threads.size(); ++i) {
            if (tmp->get_task_queue().get_task_count() > m_task_threads[i]->get_task_queue().get_task_count()) {
                tmp = m_task_threads[i];
            }
        }

        if (tmp->get_task_queue().get_task_count() != 0 && m_task_threads.size() < m_max_thread_count) {
            tmp = create_task_thread();
        }
    }
    return tmp;
}

} // namespace NSP_DD
