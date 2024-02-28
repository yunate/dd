#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include <windows.h>
#include "ddbase/ddio.h"
#include "ddbase/ddlog.hpp"
#include "ddbase/ddio.h"

namespace NSP_DD {
DDTEST(test_postmessage_to_window, postmessage)
{
    ddcout(NSP_DD::gray) << "please input the windows handle:\n";
    std::wstring hwnd;
    ddcin() >> hwnd;
    u64 wnd = 0;
    ::swscanf_s(hwnd.c_str(), L"%lld", &wnd);
    if (wnd == 0) {
        ::swscanf_s(hwnd.c_str(), L"%llX", &wnd);
    }

    while (true) {
        std::wstring message_str;
        ddcout(NSP_DD::gray) << "please input the message id\n";
        ddcin() >> message_str;
        u32 message = 0;;
        ::swscanf_s(message_str.c_str(), L"%d", &message);
        if (message == 0) {
            ::swscanf_s(message_str.c_str(), L"%X", &message);
        }

        if (::PostMessageA((HWND)wnd, message, 0, 0)) {
            ddcout(NSP_DD::gray) << "PoseMessage successful!\n";
        } else {
            ddcout(NSP_DD::gray) << "PoseMessage failure!\n";
            DDLOG_LASTERROR()
        }
    }
}

} // namespace NSP_DD
