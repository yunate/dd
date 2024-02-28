#include "test/stdafx.h"
#if _HAS_CXX20
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/thread/ddevent.h"
#include "ddbase/coroutine/ddcoroutine.h"

#include <thread>


void asyncio(const std::function<bool(bool)>& callback)
{
    // callback(false);
    std::thread([callback]() {
        //::Sleep(1000);
        callback(false);
    }).detach();
}

void asyncio1(const std::function<void(void)>& callback)
{
    // callback();
    std::thread([callback]() {
        //::Sleep(1000);
        callback();
    }).detach();
}

void asyncio2(const std::function<void(void)>& callback)
{
    callback();
}

NSP_DD::ddcoroutine<void> co_io()
{
    co_await NSP_DD::ddco_async(asyncio1);
    auto raw_callback = [](bool) { return true; };
    co_await NSP_DD::ddco_async([raw_callback](const std::function<void()>& resumer) {
        asyncio([resumer, raw_callback](bool v) {
            bool ret = raw_callback(v);
            resumer();
            return ret;
        });
    });

    auto y = NSP_DD::ddcoroutine_from(asyncio1);
    co_await y;
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
    return [ms](const std::function<void()>& resumer) {
        std::thread([ms, resumer]() {
            ::Sleep(ms);
            resumer();
        }).detach();
    };
}

NSP_DD::ddcoroutine<void> test_stackoverflow()
{
    for (int i = 0; i < 1000; ++i) {
        co_await counter();
    }

    for (int i = 0; i < 3000; ++i) {
        co_await NSP_DD::ddco_async(asyncio2);
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
    co_await test_stackoverflow();
    auto value = co_await counter(); value;
    auto value1 = co_await counter1(); value1;
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
    co_return ;
}

namespace NSP_DD {
ddcoroutine<int> wait_test(std::shared_ptr<ddevent> e)
{
    co_await test();
    DDASSERT(e != nullptr);
    e->notify();
    co_return 3;
}

DDTEST(test_case_coroutine, 1)
{
    for (size_t i = 0; i < 1; ++i) {
        std::shared_ptr<ddevent> e(new ddevent());
        auto promise = wait_test(e);
        promise.run();
        e->wait();
        auto x = promise.get_value();
        ++x;
        std::cout << i << std::endl;
    }
}
} // namespace NSP_DD
#endif // c20
