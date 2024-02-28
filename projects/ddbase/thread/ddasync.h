#ifndef ddbase_thread_ddasync_h_
#define ddbase_thread_ddasync_h_

#include "ddbase/dddef.h"
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
} // namespace NSP_DD
#endif // ddbase_thread_ddasync_h_