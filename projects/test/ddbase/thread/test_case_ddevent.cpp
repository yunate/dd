#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/thread/ddevent.h"
#include "ddbase/ddtime.h"

namespace NSP_DD {
DDTEST(test_ddevent, 1)
{
    ddevent event;
    DDASSERT(!event.wait(3000));
    std::vector<std::thread*> threads;
    for (s32 i = 0; i < 10; ++i) {
        threads.push_back(new std::thread([&event]() {
            for (s32 i = 0; i < 100; ++i) {
                event.wait();
            }
        }));
    }

    std::thread notify_thread([&event]() {
        for (s32 i = 0; i < 1000; ++i) {
            event.notify();
        }
        // event.notify_all();
    });

    for (s32 i = 0; i < 10; ++i) {
        threads[i]->join();
        delete threads[i];
    }
    notify_thread.join();

}
} // namespace NSP_DD

