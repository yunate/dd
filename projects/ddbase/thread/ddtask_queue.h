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
    ddtask();

    // 任务id
    u64 task_id();
    void set_task_id(u64 id);

    // 执行任务
    virtual void run_task() = 0;

private:
    u64 m_id = 0;
};
using sptask = std::shared_ptr<ddtask>;

class ddtimeout_task;
using sptimeout_task = std::shared_ptr<ddtimeout_task>;

class ddtask_queue
{
    DDNO_COPY_MOVE(ddtask_queue);
public:
    // max_cnt 最大任务数值MAX_U64表示无上限
    ddtask_queue(u64 max_cnt = MAX_U64);
    ~ddtask_queue();
    void set_max_count(u64);
    u64 task_count();
    void lock();
    void unlock();

    // 等待一个任务
    // @param[in] timeout 超时时间, MAX_U64 时表示永不超时
    // @return false 表示超时
    bool wait_for_task(u64 timeout = MAX_U64);
    void stop_wait();

    // 取出第一个ready的task
    sptask get_next_ready_task();
    // 执行所有的ready的task
    void run_all_ready_tasks();

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

    // 取消一个任务, 如果任务不存在则返回false
    bool die_task(u64 task_id);
    void die_all_tasks();

private:
    // max count
    bool wait_max_count_cv(u64 timeout);

    // timeout task
    u64 push_timeout_task_inner(const sptask& task, u64 push_timeout);
    bool process_timeout_task(u64 now);
    std::list<sptask>::iterator erase_from_ready_tasks(const std::list<sptask>::iterator& it);

private:
    ddmutex m_mutex;
    std::list<sptask> m_ready_tasks;
    std::list<sptask> m_running_tasks;
    ddcv m_wait_task_cv;

    // max count 为MAX_U64时候表示无限多
    u64 m_max_count = MAX_U64;
    u64 m_task_count = 0;
    ddcv m_max_count_cv;

    // timeout task
    std::list<sptask> m_timeout_tasks;
    u64 m_next_timeout_task_ready_time = MAX_U64;
    std::unordered_set<u64> m_ready_timeout_tasks;
};

} // namespace NSP_DD
#endif // ddbase_thread_ddtask_queue_h_
