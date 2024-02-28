#ifndef ddbase_ddexec_guard_hpp_
#define ddbase_ddexec_guard_hpp_

#include "ddbase/ddassert.h"
#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"

#include <functional>

namespace NSP_DD {

class ddexec_guard
{
    DDNO_COPY_MOVE(ddexec_guard);
public:
    using EXEC = std::function<void()>;
    ddexec_guard(const EXEC& exec) :
        m_exec(exec)
    {
        DDASSERT(m_exec != nullptr);
    }

    ~ddexec_guard()
    {
        if (m_exec != nullptr) {
            m_exec();
        }
    }

    void clear()
    {
        m_exec = nullptr;
    }
private:
    EXEC m_exec;
};

} // namespace NSP_DD
#endif // ddbase_ddexec_guard_hpp_
