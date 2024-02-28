#ifndef ddwin_control_ddtrue_window_h_
#define ddwin_control_ddtrue_window_h_

#include "ddwin/ddnative_window.h"
namespace NSP_DD {

class ddtrue_window : public ddnative_window
{
public:
    ddtrue_window(const std::wstring& window_name);
    ~ddtrue_window() = default;
    DDWIN_PROC_CHAIN_DEFINE(ddtrue_window, ddnative_window);
    bool win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
};

} // namespace NSP_DD
#endif // ddwin_control_ddtrue_window_h_