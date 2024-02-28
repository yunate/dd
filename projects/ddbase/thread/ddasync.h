#ifndef ddbase_thread_ddasync_h_
#define ddbase_thread_ddasync_h_

#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include <functional>
namespace NSP_DD {

using ddclosure = std::function<void()>;
using ddasync_caller = std::function<void(const ddclosure&)>;

inline void ddmaybe_async_call(const ddasync_caller& caller, const ddclosure& task)
{
    if (caller == nullptr) {
        task();
        return;
    }

    caller(task);
}

inline ddcoroutine<void> ddmaybe_co_async_call(const ddasync_caller& caller, const std::function<void()>& task)
{
    if (caller == nullptr) {
        task();
        co_return;
    }

    co_return co_await ddawaitable_ex([&caller, &task](const ddresume_helper& resumer) {
        caller([&task, resumer]() {
            resumer.lazy_resume();
            task();
        });
    });
}
} // namespace NSP_DD
#endif // ddbase_thread_ddasync_h_