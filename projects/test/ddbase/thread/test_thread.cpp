#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/thread/ddtask_thread.h"
#include "ddbase/ddtime.h"
#include "ddbase/thread/ddtask_thread.h"

#include <iostream>
#include <atomic>
namespace NSP_DD {
DDTEST(test_thread11, ddtask_thread_long_time)
{
    std::atomic_int count = 0;
    ddtask_thread task_thread;
    task_thread.start();

    for (int i = 0; i < 30; ++i) {
        auto& it = task_thread.get_task_queue();
        it.push_task([&count, i]() {
            ++count;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout << "ddtask_thread_long_time execute" << i << std::endl;
            return true;
        });
        std::cout << "ddtask_thread_long_time it.push_task" << i << std::endl;
    }

    while (true)
    {
        if (count == 30) {
            break;
        }

        // ::Sleep(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    task_thread.stop();
    std::cout << "ddtask_thread_long_time stop";
}

DDTEST(test_thread12, ddtask_thread_long_time1)
{
    std::atomic_int count = 0;
    ddtask_thread task_thread;
    task_thread.start();

    for (int i = 0; i < 30; ++i) {
        auto& it = task_thread.get_task_queue();
        it.push_task([&count, i]() {
            ++count;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            std::cout << "ddtask_thread_long_time execute" << i << std::endl;
            return true;
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (i == 12) {
            task_thread.stop();
        }
        std::cout << "ddtask_thread_long_time it.push_task" << i << std::endl;
    }

    while (true)
    {
        // ::Sleep(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    std::cout << "ddtask_thread_long_time stop";
}

DDTEST(test_thread13, simple_task_queue)
{
    ddtask_queue task_que;
    for (int i = 0; i < 30; ++i) {
        task_que.push_task([i]() {
            std::cout << "push_heartbeat_task" << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return true;
        });
    }
    task_que.push_task([&task_que]() {
        return true;
    });
    task_que.run_and_wait(0);
}

DDTEST(test_thread4, ddtask_thread_pool)
{
    std::atomic_int count = 0;
    ddtask_thread task_thread;
    task_thread.start();
    std::cout << count << ":" << ddtime::now_ms() << std::endl;

    task_thread.get_task_queue().push_task([&count]() {
        ++count;
        std::cout << count << ":" << ddtime::now_ms() << "::run timeout task:" << std::endl;
        return true;
    }, 2000, 10);
    task_thread.get_task_queue().push_task([&count]() {
        ++count;
        std::cout << count << ":" << ddtime::now_ms() << "::run timeout task:" << std::endl;
        return true;
    }, 2000, 10);
    task_thread.get_task_queue().push_task([&count]() {
        ++count;
        std::cout << count << ":" << ddtime::now_ms() << "::run timeout task:" << std::endl;
        return true;
    }, 2000, 10);

    while (true)
    {
        if (count == 30) {
            break;
        }

        // ::Sleep(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

DDTEST(test_thread5, ddthread)
{
    ddtask_thread thread;
    thread.start();

    std::atomic_int count = 0;
    auto task = [&count]() {
        ++count;
        std::cout << count << ":" << ddtime::now_ms() << "task" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return true;
    };

    auto time_task = [&count]() {
        ++count;
        std::cout << count << ":" << ddtime::now_ms() << "time_task" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return true;
    };

    for (int i = 0; i < 100; ++i) {
        thread.get_task_queue().push_task(task);
    }

    for (int i = 0; i < 10; ++i) {
        thread.get_task_queue().push_task(time_task, 100, 10);
    }

    while (true)
    {
        if (count >= 3000) {
            break;
        }

        thread.get_task_queue().push_task(task);
        thread.get_task_queue().push_task(time_task, 100, 10);

        // ::Sleep(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

DDTEST(test_thread6, ddthread)
{
    ddtask_thread thread;
    thread.start();

    std::atomic_int count = 0;
    auto time_task = [&count]() {
        ++count;
        u64 now = ddtime::now_ms();
        std::cout << count << ":" << now << "time_task" << std::endl;
        return true;
    };

    thread.get_task_queue().push_task(time_task, 1, MAX_U64);

    while (true)
    {
        if (count >= 30) {
            break;
        }

        // ::Sleep(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

DDTEST(test_task_thread_pool, ddthread)
{
    {
        ddtask_thread_pool pool(5);
        std::atomic_int count = 0;
        std::mutex mutex;
        auto time_task = [&count, &mutex]() {
            std::lock_guard<std::mutex>guard(mutex);
            ++count;
            u64 now = ddtime::now_ms();
            std::cout << count << ":" << now << "time_task" << std::endl;
            return true;
        };
        pool.push_task(time_task, 1, MAX_U64);
        pool.push_task(time_task);
        while (true) {
            if (count >= 3000) {
                pool.stop_all();
                break;
            }

            pool.push_task(time_task);
            pool.push_task(time_task, 1, 10);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
} // namespace NSP_DD

