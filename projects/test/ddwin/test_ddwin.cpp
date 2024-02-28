#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "test/res/resource.h"

#include "ddwin/control/ddtrue_window.h"
#include "ddwin/ddkeyboard.h"
#include "ddwin/ddmouse.h"
#include "ddwin/ddwindow_utils.hpp"
#include "ddwin/ddmsg_loop.h"
#include "ddbase/windows/ddmoudle_utils.h"

namespace NSP_DD {

class test_window : public ddtrue_window
{
public:
    test_window(const std::wstring& window_name) :
        ddtrue_window(window_name)
    {
    }

    DDWIN_PROC_CHAIN_DEFINE(test_window, ddtrue_window);
    bool win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT&)
    {
        if (KB.on_msg(hWnd, uMsg, wParam, lParam)) {
            return true;
        }
        if (MOUSE.on_msg(hWnd, uMsg, wParam, lParam)) {
            return true;
        }
        return false;
    }

public:
    ddkeyboard KB;
    ddmouse MOUSE;
};


DDTEST(test_ddwin, simple_native_window1)
{
    test_window nativeWin(L"test_window");
    (void)nativeWin.init_window();
    nativeWin.set_pos({ 500, 200 });
    nativeWin.set_size({ 500, 300 });
    nativeWin.set_icon(LoadIcon(ddmoudle_utils::get_moudleW(), MAKEINTRESOURCE(IDI_MAIN)));
    nativeWin.show();

    nativeWin.KB.ON_KEY_DOWN = ([&nativeWin](u8 code) {
        u8 title[2] = {0};
        title[0] = code;
        nativeWin.set_title((char*)title);
        return true;
    });

    nativeWin.KB.ON_KEY_UP = ([&nativeWin](u8 code) {
        bool isCtrl = nativeWin.KB.is_ctrl_down();
        if (isCtrl && code == 'C') {
            nativeWin.set_title(L"Reset");
        }
        return true;
    });

    nativeWin.MOUSE.ON_MOUSE = ([&nativeWin](const DDMOUSE_EVENT& ev) {
        switch (ev.type) {
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LDOWN: {
                nativeWin.set_title(ddstr::format(L"ON_LDOWN:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LUP: {
                nativeWin.set_title(ddstr::format(L"ON_LUP:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LDBCLICK: {
                nativeWin.set_title(ddstr::format(L"ON_LDBCLICK:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }

            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::RDOWN: {
                nativeWin.set_title(ddstr::format(L"ON_RDOWN:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::RUP: {
                nativeWin.set_title(ddstr::format(L"ON_RUP:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::RDBCLICK: {
                nativeWin.set_title(ddstr::format(L"ON_RDBCLICK:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }

            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MDOWN: {
                nativeWin.set_title(ddstr::format(L"ON_MDOWN:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MUP: {
                nativeWin.set_title(ddstr::format(L"ON_MUP:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MDBCLICK: {
                nativeWin.set_title(ddstr::format(L"ON_MDBCLICK:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }

            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MID_WHELL: {
                nativeWin.set_title(ddstr::format(L"ON_MID_WHELL:pt:%d,%d, del:%d", ev.relative_pos_x, ev.relative_pos_y, ev.wheel_del).c_str());
                break;
            }

            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MOVE: {
                nativeWin.set_title(ddstr::format(L"ON_MOVE:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }

            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::ENTER: {
                nativeWin.set_title(ddstr::format(L"ON_ENTER:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
            case DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LEAVE: {
                nativeWin.set_title(ddstr::format(L"ON_LEAVE:pt:%d,%d", ev.relative_pos_x, ev.relative_pos_y).c_str());
                break;
            }
        }
        return true;
    });
    ddwin_msg_loop loop;
    loop.loop();
}
} // namespace NSP_DD
