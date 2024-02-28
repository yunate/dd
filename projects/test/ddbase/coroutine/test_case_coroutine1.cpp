#include "test/stdafx.h"
#if _HAS_CXX20
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/thread/ddevent.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/ddexec_guard.hpp"
#include <thread>

static void asyncio(const std::function<void(bool)>& callback)
{
    std::thread([callback]() {
        callback(false);
        std::cout << "2" << std::endl;
    }).detach();
}

static NSP_DD::ddcoroutine<void> test()
{
    co_await NSP_DD::ddawaitable([](const NSP_DD::ddresume_helper& resumer) {
        asyncio([resumer](bool) {
            std::cout << "1" << std::endl;
            resumer.lazy_resume();
        });
    });
    std::cout << "3" << std::endl;
    co_return;
}

namespace NSP_DD {
void never_callback(const std::function<void(void)>&)
{
}

ddcoroutine<void> co_never_callback()
{
    auto raw_callback = [](bool) { return true; };
    co_await ddawaitable([raw_callback](const ddresume_helper& resumer) {
        never_callback([resumer, raw_callback]() {
            resumer.lazy_resume();
        });
    });

    co_return;
}

ddcoroutine<void> wait_test(std::shared_ptr<ddevent> e)
{
    ddcoroutine_run(co_never_callback());
    co_await test();
    DDASSERT(e != nullptr);
    e->notify();
    co_return;
}

DDTEST(test_case_coroutine1, 1)
{
    std::shared_ptr<ddevent> e(new ddevent());
    ddcoroutine_run(wait_test(e));
    e->wait();
}
} // namespace NSP_DD

#endif // c20