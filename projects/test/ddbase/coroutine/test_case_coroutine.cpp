#include "test/stdafx.h"
#if _HAS_CXX20
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/thread/ddevent.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/thread/ddtask_thread.h"
#include "ddbase/ddtime.h"
#include "ddbase/ddio.h"
#include <thread>

void asyncio(const std::function<bool(bool)>& callback)
{
    // callback(false);
    std::thread([callback]() {
        //::Sleep(1000);
        callback(false);
    }).detach();
}

NSP_DD::ddcoroutine<void> co_io()
{
    auto raw_callback = [](bool) { return true; };
    co_await NSP_DD::ddawaitable([raw_callback](const NSP_DD::ddresume_helper& resumer) {
        asyncio([resumer, raw_callback](bool v) {
            bool ret = raw_callback(v);
            resumer.lazy_resume();
            return ret;
        });
    });

    co_return;
}

NSP_DD::ddcoroutine<int> counter()
{
    int ii = 0;
    ++ii;
    int i = 0;
    ++i;
    co_return 1;
}

NSP_DD::ddcoroutine<std::string> counter1()
{
    int ii = 0;
    ++ii;
    int i = 0;
    ++i;
    co_return "abd";
}

NSP_DD::ddco_task create_sleep_async(NSP_DD::s32 ms)
{
    return [ms](const NSP_DD::ddresume_helper& resumer) {
        std::thread([ms, resumer]() {
            ::Sleep(ms);
            resumer.lazy_resume();
        }).detach();
    };
}

void func_callbackex(const std::function<void()>& callback)
{
    callback();
}

static void async_caller(const std::function<void()>& task)
{
    static NSP_DD::ddtask_thread_pool g_thread_pool{ 8 };
    g_thread_pool.push_task(task);
}

void func_callback(const std::function<void()>& callback)
{
    async_caller(callback);
}

NSP_DD::ddcoroutine<void> test_any1()
{
    std::vector<NSP_DD::ddcoroutine<void>> r{
        ddcoroutine_from(create_sleep_async(5000)),
        ddcoroutine_from(create_sleep_async(4000)),
        ddcoroutine_from(create_sleep_async(3000)),
        ddcoroutine_from(create_sleep_async(2000)),
        ddcoroutine_from(create_sleep_async(1000)),
        ddcoroutine_from(counter())
    };
    auto index = co_await NSP_DD::ddcoroutine_any(r, true);
    DDASSERT(index == r.size() - 1);
}

NSP_DD::ddcoroutine<void> test_any()
{
    std::vector<NSP_DD::ddcoroutine<void>> r{
        ddcoroutine_from(create_sleep_async(5000)),
        ddcoroutine_from(create_sleep_async(4000)),
        ddcoroutine_from(create_sleep_async(3000)),
        ddcoroutine_from(create_sleep_async(2000)),
        ddcoroutine_from(create_sleep_async(1000)),
        ddcoroutine_from(counter())
    };

    {
        auto index = co_await NSP_DD::ddcoroutine_any(r, false);
        DDASSERT(index == r.size() - 1);
        r.erase(r.begin() + index, r.begin() + index + 1);
    }

    {
        auto index = co_await NSP_DD::ddcoroutine_any(r, false);
        DDASSERT(index == r.size() - 1);
        r.erase(r.begin() + index, r.begin() + index + 1);
    }

    {
        auto index = co_await NSP_DD::ddcoroutine_any(r, false);
        DDASSERT(index == r.size() - 1);
        r.erase(r.begin() + index, r.begin() + index + 1);
    }

    {
        auto index = co_await NSP_DD::ddcoroutine_any(r, false);
        DDASSERT(index == r.size() - 1);
        r.erase(r.begin() + index, r.begin() + index + 1);
    }

    {
        auto index = co_await NSP_DD::ddcoroutine_any(r, false);
        DDASSERT(index == r.size() - 1);
        r.erase(r.begin() + index, r.begin() + index + 1);
    }

    {
        auto index = co_await NSP_DD::ddcoroutine_any(r, false);
        DDASSERT(index == r.size() - 1);
        r.erase(r.begin() + index, r.begin() + index + 1);
    }
}


NSP_DD::ddcoroutine<void> test_stackoverflow()
{
    NSP_DD::ddtimer timer;
    for (int i = 0; i < 3000; ++i) {
        co_await NSP_DD::ddawaitable([](const NSP_DD::ddresume_helper& resumer) {
            func_callback([resumer]() {
                resumer.lazy_resume();
            });
        });
    }

    NSP_DD::ddcout(NSP_DD::ddconsole_color::cyan) << NSP_DD::ddstr::format("ddawaitable cost:%d \n", timer.get_time_pass() / 1000000);
    timer.reset();

    for (int i = 0; i < 3000; ++i) {
        co_await NSP_DD::ddawaitable_ex([](const NSP_DD::ddresume_helper& resumer) {
            func_callbackex([resumer]() {
                resumer.lazy_resume();
            });
        });
    }

    NSP_DD::ddcout(NSP_DD::ddconsole_color::cyan) << NSP_DD::ddstr::format("ddawaitable_ex cost:%d \n", timer.get_time_pass() / 1000000);
    timer.reset();

    for (int i = 0; i < 1000; ++i) {
        co_await counter();
    }

    for (int i = 0; i < 3000; ++i) {
        co_await NSP_DD::ddcoroutine_all({
            NSP_DD::ddcoroutine_from(counter()),
            NSP_DD::ddcoroutine_from(counter1()),
            NSP_DD::ddcoroutine_from(counter1()),
            NSP_DD::ddcoroutine_from(counter1()),
            });
    }
}

NSP_DD::ddcoroutine<void> test()
{
    co_await test_any1();
    co_await test_any();
    co_await test_stackoverflow();
    auto value = co_await counter(); value;
    std::string value1 = co_await counter1(); value1;
    co_await co_io();
    co_await co_io();
    auto x = NSP_DD::ddcoroutine_from(counter());
    co_await x;
    auto y = NSP_DD::ddcoroutine_all({
        co_io(),
        NSP_DD::ddcoroutine_from(counter()),
        NSP_DD::ddcoroutine_from(counter1()),
        NSP_DD::ddcoroutine_from(create_sleep_async(3000)),
        NSP_DD::ddcoroutine_from(create_sleep_async(1000)),
        NSP_DD::ddcoroutine_from(create_sleep_async(1000)),
        NSP_DD::ddcoroutine_from(create_sleep_async(1000)),
        NSP_DD::ddcoroutine_from(create_sleep_async(1000))
        });
    co_await y;
    co_return;
}

namespace NSP_DD {
ddcoroutine<int> wait_test(std::shared_ptr<ddevent> e)
{
    co_await test();
    DDASSERT(e != nullptr);
    e->notify();
    co_return 3;
}

ddcoroutine<void> wait_test1(std::shared_ptr<ddevent> e)
{
    co_await test();
    DDASSERT(e != nullptr);
    e->notify();
    co_return;
}

class A {
public:
    A() {
        DDLOG(WARNING, "A\r\n");
    }

    ~A() {
        DDLOG(WARNING, "~A------------------------------\r\n");
    }
};

ddcoroutine<void> wait_test2()
{
    auto co1 = ddcoroutine_empty<void>();
    {
        A a1;
        co1 = ddcoroutine_from(create_sleep_async(1000));
    }
    co_await co1;

    DDLOG(WARNING, "1------------------------------\r\n");

    A a2;
    co_await ddcoroutine_from(create_sleep_async(1000));
    DDLOG(WARNING, "2------------------------------\r\n");

    A a3;
    co_return;
}

ddcoroutine<void> wait_test3() {
    auto x = wait_test2();
    co_await x;
    DDLOG(WARNING, "wait_test3------------------------------\r\n");
}

DDTEST(test_case_coroutine, 1)
{
    ddcoroutine_run(wait_test3());
    for (size_t i = 0; i < 1; ++i) {
        std::shared_ptr<ddevent> e(new ddevent());
        ddcoroutine_run(wait_test1(e));
        e->wait();

        auto promise = wait_test(e);
        e->wait();
        auto x = promise.get_value();
        ++x;
        std::cout << i << std::endl;
    }
}
} // namespace NSP_DD

#endif // c20