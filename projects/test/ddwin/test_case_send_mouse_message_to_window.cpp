#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddtime.h"
#include "ddwin/ddwindow_utils.hpp"
#include "ddbase/windows/ddprocess.h"
#include "ddbase/ddio.h"
#include <windows.h>

namespace NSP_DD {

class EnumWindowsHandler
{
public:
    EnumWindowsHandler(std::wstring process_name) :
        m_process_name(process_name) {}
    
    bool on_enum_window_callback(HWND hwnd)
    {
        if (!is_wnd_in_target_process(hwnd)) {
            return true;
        }

        if (!send_msg_to_wnd(hwnd)) {
            ddcout(ddconsole_color::red) << ddstr::format(L"send message to hwnd:0x%X failure!\r\n", hwnd);
        } else {
            ddcout(ddconsole_color::green) << ddstr::format(L"send message to hwnd:0x%X successful!\r\n", hwnd);
        }

        return true;
    }

    bool retrive_find_process_id()
    {
        m_foreground_wnd = ::GetForegroundWindow();
        return ddprocess::get_process_ids(m_process_name, m_find_process_ids);
    }

private:
    bool is_wnd_in_target_process(HWND hwnd)
    {
        if (hwnd == m_foreground_wnd) {
            return false;
        }

        DWORD process_id = 0;
        (void)::GetWindowThreadProcessId(hwnd, &process_id);
        if (process_id == 0) {
            return false;
        }

        for (auto it : m_find_process_ids) {
            if (it == process_id) {
                return true;
            }
        }
        return false;
    }

    bool send_msg_to_wnd(HWND hwnd)
    {
        return !!::PostMessage(hwnd, WM_MOUSEMOVE, 0, 0);
    }

    std::vector<DWORD> m_find_process_ids;
    std::wstring m_process_name;
    HWND m_foreground_wnd = NULL;
};

BOOL CALLBACK enum_windows_cb(HWND hwnd, LPARAM lp)
{
    return ((EnumWindowsHandler*)(lp))->on_enum_window_callback(hwnd);
}

DDTEST(test_case_send_mouse_message_to_window, 1)
{
    std::wstring process_name = L"msrdc.exe";
    EnumWindowsHandler handler(process_name);
    ddtimer timer;
    while (true) {
        u64 time_pass = timer.get_time_pass() / 1000 / 1000 / 1000;
        if (time_pass < 10) {
            continue;
        }

        timer.reset();
        if (!handler.retrive_find_process_id()) {
            ddcout(ddconsole_color::red) << ddstr::format(L"can not find %s process!\r\n", process_name.c_str());
            continue;
        }

        ddcout(ddconsole_color::blue) << ddstr::format(L"timer===>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
        (void)::EnumWindows(enum_windows_cb, (LPARAM)&handler);
        ::Sleep(1000);
    }
}
} // namespace NSP_DD
