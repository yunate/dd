
#ifndef ddbase_thread_ddevent_h_
#define ddbase_thread_ddevent_h_

#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"
#include "ddbase/ddnocopyable.hpp"
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace NSP_DD {

////////////////////////////////////ddmutex////////////////////////////////////
class ddcv;
class ddmutex
{
    friend ddcv;
    DDNO_COPY_MOVE(ddmutex);
public:
    ddmutex() = default;
    ~ddmutex() = default;
    void lock();
    void unlock();
    bool try_lock();

    // @return 是否获得锁
    bool try_lock_for(u64 ms);
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    u32 m_locked_count = 0;
    std::thread::id m_thread_id;
};

////////////////////////////////////ddcv////////////////////////////////////
class ddcv
{
    DDNO_COPY_MOVE(ddcv);
public:
    ddcv() = default;
    ~ddcv() = default;
    void notify_one();
    void notify_all();
    void wait(ddmutex& mutex);

    // @return 是否获得锁
    bool wait_for(ddmutex& mutex, u64 ms);
private:
    std::condition_variable_any m_cv;
};

////////////////////////////////////ddevent////////////////////////////////////
/** 简单事件
@note 不提供析构函数，如果当该类析构的时候，但是还处于Wait() 状态，很不幸的告诉你发生未定义行为
*/
class ddevent
{
    DDNO_COPY_MOVE(ddevent);
public:
    ddevent() = default;
    ~ddevent() = default;

    // 等待一个信号
    // @param[in] timeout 超时时间，为MAX_U64时表示永不超时
    // @return 超时返回false，否者true
    bool wait(u64 timeout = MAX_U64);

    void notify_one();

    // 调用该函数侯，所有的wait 都被通知，未来调用的wait 也不会等待，除非调用reset()函数恢复状态
    void notify_all();

    void reset(bool signal);
private:
    std::mutex m_mutex;
    std::condition_variable m_con;

    /** 是否有信号，只是记录是否有信号，但是不记录信号次数
    @note 主要是为了先调用SetEvent()，然后调用wait的情况。考虑在Wait()前加上::Sleep(1000);
    */
    bool m_hasSignal = false;

    // notify all 时候的标记
    bool m_notyfiall = false;
};

////////////////////////////////////ddsemaphore////////////////////////////////////
class ddsemaphore
{
    DDNO_COPY_MOVE(ddsemaphore);
public:
    ddsemaphore() = default;
    ~ddsemaphore() = default;

    // 等待一个信号
    // @param[in] timeout 超时时间，为MAX_U64时表示永不超时
    // @return 超时返回false，否者true
    bool wait(u64 timeout = MAX_U64);

    void notify_one();

    // 调用该函数侯，所有的wait 都被通知，未来调用的wait 也不会等待，除非调用reset()函数恢复状态
    void notify_all();

    void reset(u32 cnt);

private:
    std::mutex m_mutex;
    std::condition_variable m_con;

    /** 是否有信号
    @note 主要是为了先调用SetEvent()，然后调用wait() 不会被唤醒的情况。考虑在Wait()前加上::Sleep(1000);
    */
    long m_count = 0;

    // notify all 时候的标记
    bool m_notyfiall = false;
};

////////////////////////////////////spin_mutex////////////////////////////////////
class ddspin_mutex
{
    DDNO_COPY_MOVE(ddspin_mutex);
public:
    ddspin_mutex() = default;
    ~ddspin_mutex() = default;


public:
    void lock();
    void unlock();

private:
    std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};

} // namespace NSP_DD
#endif // ddbase_thread_ddevent_h_

