#ifndef ddwin_ddmsg_loop_h_
#define ddwin_ddmsg_loop_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include <windows.h>
namespace NSP_DD {
class ddwin_msg_loop
{
    DDNO_COPY_MOVE(ddwin_msg_loop);
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
    u32 m_loop_id = 0;
};

} // namespace NSP_DD
#endif // ddwin_ddmsg_loop_h_