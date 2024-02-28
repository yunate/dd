#ifndef ddbase_thread_ddtask_thread_h_
#define ddbase_thread_ddtask_thread_h_

#include "ddbase/thread/ddtask_queue.h"
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
    bool start();
    // kill all task and stop
    bool stop();
private:
    volatile bool m_stop = false;
    ddtask_queue m_task_queue;
    std::thread* m_thread = nullptr;
    std::recursive_mutex m_mutex;
};

//////////////////////////////ddtask_thread_pool//////////////////////////////
class ddtask_thread_pool
{
    DDNO_COPY_MOVE(ddtask_thread_pool);
public:
    ddtask_thread_pool(u32 thread_count, u32 pre_thread_task_count = MAX_U32);
    ~ddtask_thread_pool();
    void stop_all();
    bool kill_task(u64 task_id);
    void kill_all_tasks();
    u64 push_task(const std::function<void()>& task, u64 task_timeout = 0, u64 run_times = 1);
    u64 push_task(const std::shared_ptr<ddtask>& task, u64 task_timeout = 0, u64 run_times = 1);

    // @return false 表示超时
    bool wait_ready_to_push(u64 timeout = MAX_U64);

    // 等待pool中的task都运行结束
    // @note 不会关注run_times不为1的任务, 所以即使这个函数返回true, 依然会有未执行的任务.
    bool wait_end(u64 timeout = MAX_U64);

private:
    std::shared_ptr<ddtask_thread> create_task_thread();
    std::shared_ptr<ddtask_thread> get_task_thread();
    std::recursive_mutex m_mutex;
    std::vector<std::shared_ptr<ddtask_thread>> m_task_threads;
    u32 m_pre_thread_task_count = MAX_U32;
    u32 m_max_thread_count = 0;
    ddevent m_event;
};
} // namespace NSP_DD
#endif // ddbase_thread_ddtask_thread_h_

