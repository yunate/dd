#ifndef ddbase_thread_ddtask_thread_h_
#define ddbase_thread_ddtask_thread_h_

#include "ddbase/thread/ddtask_queue.h"
#include "ddbase/thread/ddloop.h"
#include <atomic>
#include <thread>
namespace NSP_DD {

//////////////////////////////ddtask_thread//////////////////////////////
class ddtask_thread
{
    DDNO_COPY_MOVE(ddtask_thread);
public:
    ddtask_thread() = default;
    ~ddtask_thread();
    ddtask_queue& get_task_queue();
    void join();
    bool stop(u64 waitTime);
    bool start();
    bool has_start();
private:
    ddfunction_loop* m_loop = nullptr;
    bool m_stop = false;
    ddtask_queue m_task_queue;
    std::thread* m_thread = nullptr;
    std::recursive_mutex m_mutex;
};

//////////////////////////////ddtask_thread_pool//////////////////////////////
class ddtask_thread_pool
{
    ddtask_thread_pool(ddtask_thread);
public:
    ddtask_thread_pool(u32 thread_count);
    ~ddtask_thread_pool();
    virtual void stop_all();

    // 向任务队列里压入任务
    // @param[in] task 任务
    // @param[in] timeout 超时时间(ms), 在timeout时间后执行任务,为0时候立刻执行
    // @param[in] times 执行times次数后删除,为MAX_U64时候一直不删除
    // @param[in] timeout 队列满了的时候的等待时间, MAX_U64表示一直等待不超时
    // @return task id, 为0时表示失败
    u64 push_task(const sptask& task, u64 push_timeout = MAX_U64);
    u64 push_task(const ddclosure& task, u64 push_timeout = MAX_U64);
    u64 push_task(const sptask& task, u64 task_timeout, u64 times, u64 push_timeout = MAX_U64);
    u64 push_task(const ddclosure& task, u64 task_timeout, u64 times, u64 push_timeout = MAX_U64);
    bool die_task(u64 task_id);
    void die_all_tasks();

private:
    std::shared_ptr<ddtask_thread> get_task_thread();
private:
    std::vector<std::shared_ptr<ddtask_thread>> m_task_threads;
    u32 m_max_count = 0;
    ddmutex m_mutex;
};
} // namespace NSP_DD
#endif // ddbase_thread_ddtask_thread_h_

