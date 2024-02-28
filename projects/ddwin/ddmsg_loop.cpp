#include "ddwin/stdafx.h"
#include "ddwin/ddmsg_loop.h"
namespace NSP_DD {
bool ddwin_msg_loop::stop(u64)
{
    if (::PostThreadMessage(m_loop_id, WM_QUIT, NULL, NULL)) {
        m_stop = true;
    }
    return m_stop;
}

void ddwin_msg_loop::loop()
{
    m_loop_id = ::GetCurrentThreadId();
    while (!m_stop) {
        BOOL hr = ::GetMessage(&m_msg, NULL, 0, 0);
        if (hr == 0) {
            // WM_QUIT, exit message loop
            m_stop = true;
            continue;
        } else if (hr == -1) {
            continue;
        }

        if (!filter_msg(&m_msg)) {
            ::TranslateMessage(&m_msg);
            ::DispatchMessage(&m_msg);
        }
    }
}

bool ddwin_msg_loop::filter_msg(MSG* msg)
{
    (msg);
    return false;
}

} // namespace NSP_DD
