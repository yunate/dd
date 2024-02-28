#include "ddwin/stdafx.h"
#include "ddwin/ddnative_window.h"
#include "ddwin/ddwindow_utils.hpp"
#include "ddbase/windows/ddmoudle_utils.h"
namespace NSP_DD {

static LRESULT CALLBACK global_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 获得实列对象，处理窗口事件
    ddnative_window* nw = (ddnative_window*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (nw != NULL) {
        LRESULT result = S_OK;
        if (nw->win_proc(hWnd, uMsg, wParam, lParam, result)) {
            return result;
        }
    } else if (WM_CREATE == uMsg) {
        (void)::SetWindowLongPtr(hWnd, GWLP_USERDATA, (DWORD_PTR)(((CREATESTRUCT*)lParam)->lpCreateParams));
        return ::SendMessage(hWnd, uMsg, wParam, lParam); // 将WM_CREATE消息抛给下一次消息循环
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

const wchar_t* ddnative_window::get_class_name()
{
    return L"ddnative_window";
}

ddnative_window::ddnative_window(const std::wstring& title) :
    m_title(title)
{
}
ddnative_window::~ddnative_window()
{
}

bool ddnative_window::register_window_class()
{
    static bool has_registered = false;
    if (!has_registered) {
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX),0 };
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_DBLCLKS;
        wcex.lpfnWndProc = global_proc;
        wcex.hInstance = ddmoudle_utils::get_moudleW();
        wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = get_class_name();
        has_registered = !!::RegisterClassEx(&wcex);
    }
    return has_registered;
}

bool ddnative_window::win_proc(HWND hWnd, UINT uMsg, WPARAM w, LPARAM l, LRESULT& result)
{
    switch (uMsg) {
        case WM_SIZE:
        case WM_MOVE:
        {
            RECT rect;
            ddwindow_utils::get_rect(hWnd, rect);
            m_size.x = rect.right - rect.left;
            m_size.y = rect.bottom - rect.top;
            m_pos.x = rect.left;
            m_pos.y = rect.top;
            break;
        }
        default:
        {
            break;
        }
    }

    return DDWIN_PROC_CHAIN(hWnd, uMsg, w, l, result);
}

bool ddnative_window::init_window()
{
    if (m_hWnd != NULL) {
        return true;
    }

    if (!register_window_class()) {
        return false;
    }

    m_hWnd = ::CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
        get_class_name(), m_title.c_str(),
        WS_OVERLAPPEDWINDOW,
        // WS_POPUP,
        0, 0, 0, 0,
        0, 0, ddmoudle_utils::get_moudleW(),
        this);
    return m_hWnd != NULL;
}

HWND ddnative_window::get_window()
{
    return m_hWnd;
}

ddpoint ddnative_window::get_size()
{
    return m_size;
}

void ddnative_window::set_size(const ddpoint& size)
{
    DDASSERT(m_hWnd != NULL);
    (void)ddwindow_utils::set_size(m_hWnd, size.x, size.y);
}

ddpoint ddnative_window::get_pos()
{
    return m_pos;
}

void ddnative_window::set_pos(const ddpoint& pos)
{
    DDASSERT(m_hWnd != NULL);
    (void)ddwindow_utils::set_pos(m_hWnd, pos.x, pos.y);
}

void ddnative_window::show()
{
    DDASSERT(m_hWnd != NULL);
    ddwindow_utils::show(m_hWnd, true);
}

void ddnative_window::hide()
{
    DDASSERT(m_hWnd != NULL);
    ddwindow_utils::show(m_hWnd, false);
}

std::wstring ddnative_window::get_title()
{
    return m_title;
}

void ddnative_window::set_title(const std::wstring& title)
{
    m_title = title;
    DDASSERT(m_hWnd != NULL);
    ddwindow_utils::set_title(m_hWnd, m_title.c_str());
}

void ddnative_window::set_title(const std::string& title)
{
    m_title = ddstr::ansi_utf16(title);
    DDASSERT(m_hWnd != NULL);
    ddwindow_utils::set_title(m_hWnd, m_title.c_str());
}

void ddnative_window::set_icon(HICON icon)
{
    DDASSERT(m_hWnd != NULL);
    (void)::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    (void)::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
}

} // namespace NSP_DD
