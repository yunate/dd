#include "ddwin/stdafx.h"
#include "ddwin/control/ddtrue_window.h"

namespace NSP_DD {

ddtrue_window::ddtrue_window(const std::wstring& window_name)
    : ddnative_window(window_name)
{
}

bool ddtrue_window::win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    if (DDWIN_PROC_CHAIN(hWnd, uMsg, wParam, lParam, result)) {
        return true;
    }

    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hWnd, &ps);
            ::FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            ::MoveToEx(hdc, 100, 300, NULL);
            ::LineTo(hdc, 200, 400);
            EndPaint(hWnd, &ps);
            return true;
        }
    }

    return false;
}

} // namespace NSP_DD
