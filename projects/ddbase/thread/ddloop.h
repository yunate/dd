#ifndef ddbase_thread_ddloop_h_
#define ddbase_thread_ddloop_h_

#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"
#include "ddbase/thread/ddevent.h"
#include "ddbase/ddnocopyable.hpp"
#include <functional>
#include <windows.h>
namespace NSP_DD {

class ddiloop
{
    DDNO_COPY_MOVE(ddiloop);
public:
    ddiloop() = default;
    virtual ~ddiloop() = default;
    virtual void loop() = 0;
    virtual bool stop(u64 timeout) = 0;
};

//////////////////////////////ddloop//////////////////////////////
class ddloop : public ddiloop
{
public:
    ddloop() = default;
    virtual ~ddloop();

public:
    virtual bool stop(u64 timeout);
    virtual void loop();

protected:
    virtual void loop_core() = 0;

protected:
    std::recursive_timed_mutex m_mutex;

private:
};
//////////////////////////////ddfunction_loop//////////////////////////////
class ddfunction_loop : public ddloop
{
public:
    ddfunction_loop(const std::function<void()>& callback);
protected:
    void loop_core() override;
private:
    std::function<void()> m_callback;
};
//////////////////////////////ddwin_msg_loop//////////////////////////////
class ddwin_msg_loop : public ddiloop
{
public:
    ddwin_msg_loop() = default;
    virtual ~ddwin_msg_loop() = default;

public:
    virtual bool stop(u64 timeout);
    virtual void loop();

    // 返回true则该条消息被过滤
    virtual bool filter_msg(MSG* msg);

protected:
    bool m_stop = false;
    MSG m_msg{ 0 };
    u32 m_loopId = 0;
};

} // namespace NSP_DD
#endif // ddbase_thread_ddloop_h_