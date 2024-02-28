#ifndef ddbase_ddlazy_exec_h_
#define ddbase_ddlazy_exec_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include <mutex>
#include <list>
#include <functional>

namespace NSP_DD {

// 保证所有的执行体一定在调用了 ready() 之后执行
class ddlazy_exec
{
    DDNO_COPY_MOVE(ddlazy_exec);
public:
    ddlazy_exec() = default;
    ~ddlazy_exec() = default;

    using lazy_excutable = std::function<void()>;
    inline void exec(const lazy_excutable& excutable)
    {
        m_mutex.lock();
        if (m_ready) {
            m_mutex.unlock();
            excutable();
            return;
        }

        m_execs.push_back(excutable);
        m_mutex.unlock();
    }

    inline void ready(bool execAll)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            m_ready = true;
        }

        if (execAll) {
            for (const auto& it : m_execs) {
                it();
            }
        }
        m_execs.clear();
    }

    inline void reset()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_ready = false;
        m_execs.clear();
    }

private:
    std::recursive_mutex m_mutex;
    std::list<lazy_excutable> m_execs;
    bool m_ready = false;
};

} // namespace NSP_DD
#endif // ddbase_ddlazy_exec_h_
