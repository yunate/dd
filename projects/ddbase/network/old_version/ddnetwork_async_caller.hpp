#ifndef ddbase_network_ddnetwork_async_caller_hpp_
#define ddbase_network_ddnetwork_async_caller_hpp_
#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include <functional>

namespace NSP_DD {

class ddnetwork_async_caller
{
public:
    // should set the caller at the beginning to make sure the s_caller thread safe.
    static void set_caller(const std::function<void(const std::function<void()>&)>& caller)
    {
        s_caller = caller;
        s_has_set_async = true;
    }

    // call the task in current if never set the caller
    // or call the task by the setted caller.
    static void call(const std::function<void()>& task)
    {
        DDASSERT_FMT(s_has_set_async, L"should call set_caller first to set the caller.");
        // if never set the caller, call the task in current thread
        if (s_caller == nullptr) {
            task();
            return;
        }

        s_caller(task);
    }
private:
    static std::function<void(const std::function<void()>&)> s_caller;
    static bool s_has_set_async;
};

#define DDNETWORK_ASYNC_CALL ddnetwork_async_caller::call
} // namespace NSP_DD
#endif // ddbase_network_ddnetwork_async_caller_hpp_
