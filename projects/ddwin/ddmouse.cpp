#include "ddwin/stdafx.h"
#include "ddwin/ddmouse.h"
#include "ddwin/ddwindow_utils.hpp"
namespace NSP_DD {

#define MAKE_BOOL(val) ((val) != 0)

bool ddmouse::is_key_down(int key)
{
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    return (::GetAsyncKeyState(key) & 0x8000);
}

bool ddmouse::is_ldown()
{
    return is_key_down(VK_LBUTTON);
}

bool ddmouse::is_rdown()
{
    return is_key_down(VK_RBUTTON);
}

bool ddmouse::is_mdown()
{
    return is_key_down(VK_MBUTTON);
}

bool ddmouse::on_msg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DDMOUSE_EVENT me;
    switch (msg) {
        // L_BUTTON
        case WM_LBUTTONDOWN: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LDOWN;
            break;
        }
        case WM_LBUTTONUP: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LUP;
            break;
        }
        case WM_LBUTTONDBLCLK: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LDBCLICK;
            break;
        }

        // R_BUTTON
        case WM_RBUTTONDOWN: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::RDOWN;
            break;
        }
        case WM_RBUTTONUP: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::RUP;
            break;
        }
        case WM_RBUTTONDBLCLK: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::RDBCLICK;
            break;
        }

        // M_BUTTON
        case WM_MBUTTONDOWN: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MDOWN;
            break;
        }
        case WM_MBUTTONUP: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MUP;
            break;
        }
        case WM_MBUTTONDBLCLK: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MDBCLICK;
            break;
        }
        case WM_MOUSEWHEEL: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MID_WHELL;
            me.wheel_del = (s32)(GET_WHEEL_DELTA_WPARAM(wParam));
            break;
        }

        // MOVE
        case WM_MOUSEMOVE: {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MOVE;
            break;
        }

        case WM_MOUSELEAVE: {
            // 用于重叠窗口
            if (m_captured) {
                (void)::ReleaseCapture();
                m_captured = false;
                me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LEAVE;
            }
            break;
        }
    }

    if (me.type == DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::NONE) {
        return false;
    }

    WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
    me.is_ldown = MAKE_BOOL(MK_LBUTTON & fwKeys);
    me.is_rdown = MAKE_BOOL(MK_RBUTTON & fwKeys);
    me.is_mdown = MAKE_BOOL(MK_MBUTTON & fwKeys);

    if (me.type != DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LEAVE) {
        const POINTS pt = MAKEPOINTS(lParam);
        me.relative_pos_x = (s32)pt.x;
        me.relative_pos_y = (s32)pt.y;

        if (me.type == DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::MID_WHELL) {
            POINT to{ me.relative_pos_x, me.relative_pos_y};
            ::ScreenToClient(hWnd, &to);
            me.relative_pos_x = to.x;
            me.relative_pos_y = to.y;
        }

        // 这个必须要, 因为从client到标题栏WM_MOUSELEAVE消息不会收到
        if (try_leave(hWnd, me)) {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::LEAVE;
            return call_cbs(me);
        }

        if (try_enter(hWnd, me)) {
            me.type = DDMOUSE_EVENT::DDMOUSE_EVENT_TYPE::ENTER;
            return call_cbs(me);
        }
    }

    return call_cbs(me);
}

bool ddmouse::call_cbs(const DDMOUSE_EVENT& me)
{
    if (ON_MOUSE == nullptr) {
        return false;
    }
    return ON_MOUSE(me);
}

bool ddmouse::try_leave(HWND hwnd, const DDMOUSE_EVENT& me)
{
    if (!m_captured) {
        return false;
    }

    if (me.is_ldown || me.is_mdown || me.is_rdown) {
        return false;
    }

    RECT rect;
    ddwindow_utils::get_client_rect(hwnd, rect);
    s32 width = rect.right - rect.left;
    s32 height = rect.bottom - rect.top;
    s32 x = me.relative_pos_x;
    s32 y = me.relative_pos_y;
    if (x < 0 || x > width || y < 0 || y > height) {
        m_captured = false;
        ::ReleaseCapture();
        return true;
    }
    return false;
}

bool ddmouse::try_enter(HWND hwnd, const DDMOUSE_EVENT& me)
{
    if (m_captured) {
        return false;
    }

    RECT rect;
    ddwindow_utils::get_client_rect(hwnd, rect);
    s32 width = rect.right - rect.left;
    s32 height = rect.bottom - rect.top;
    s32 x = me.relative_pos_x;
    s32 y = me.relative_pos_y;
    if (x >= 0 && x <= width && y >= 0 && y <= height) {
        ::SetCapture(hwnd);
        TRACKMOUSEEVENT trackMouseEvent;
        trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
        trackMouseEvent.dwFlags = TME_LEAVE;
        trackMouseEvent.hwndTrack = hwnd;
        trackMouseEvent.dwHoverTime = 0;
        (void)::TrackMouseEvent(&trackMouseEvent);
        m_captured = true;
        return true;
    }
    return false;
}


} // namespace NSP_DD
