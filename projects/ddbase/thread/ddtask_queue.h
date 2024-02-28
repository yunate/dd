#ifndef ddbase_thread_ddtask_queue_h_
#define ddbase_thread_ddtask_queue_h_

#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/thread/ddasync.h"
#include "ddbase/thread/ddevent.h"
#include <functional>
#include <list>
#include <memory>
#include <unordered_set>

namespace NSP_DD {
class ddtask
{
public:
    virtual ~ddtask() = default;
    virtual void run_task() = 0;
};

class ddtask_queue
{
    DDNO_COPY_MOVE(ddtask_queue);
public:
    // max_count 最大任务数
    // on_any_task_complete, 当任务完成时候触发, 触发时表示队列有空闲, 可以继续push
    ddtask_queue(u32 max_count = MAX_U32, const std::function<void(u32 count)>& on_any_task_complete = nullptr);
    ~ddtask_queue();
    void set_max_count(u32 max_count);
    void set_any_task_complete_handler(const std::function<void(u32 count)>& on_any_task_complete);
    u32 get_max_count();
    u64 get_task_count();

    // 执行并等待这个队列, 可以在循环中反复执行
    // @note 该函数只能同时在一个线程中执行
    // @param[in] timeout 超时时间, MAX_U64 时表示永不超时
    // @return false 表示超时
    bool run_and_wait(u64 timeout = MAX_U64);

    // 向任务队列里压入任务
    // @param[in] task 任务
    // @param[in] task_timeout 超时时间(ms), 在timeout时间后执行任务,为0时候立刻执行
    // @param[in] run_times 执行times次数后删除,为MAX_U64时候一直不删除
    // @return task id, 为0时表示失败
    u64 push_task(const std::function<void()>& task, u64 task_timeout = 0, u64 run_times = 1);
    u64 push_task(const std::shared_ptr<ddtask>& task, u64 task_timeout = 0, u64 run_times = 1);

    // 向队列里面压入一个特殊的task, 当其执行或者被kill的时候, 返回的ddevent会被notify
    std::shared_ptr<ddevent> push_flag_task();

    // 取消一个还未执行的任务, 如果任务不存在则返回false
    bool kill_task(u64 task_id);
    void kill_all_tasks();

private:
    void notify();

#ifdef _DEBUG
    std::atomic_bool m_run_and_waiting = false;;
#endif
    std::mutex m_mutex;
    ddcv m_cv;
    bool m_noifyed = false;
    std::list<ddtask*> m_tasks;
    std::list<ddtask*> m_ready_tasks;
    u32 m_max_count = MAX_U32;
    std::function<void(u32 count)> m_on_any_task_complete;
};

} // namespace NSP_DD
#endif // ddbase_thread_ddtask_queue_h_
