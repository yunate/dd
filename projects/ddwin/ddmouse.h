
#ifndef ddwin_ddmouse_h_
#define ddwin_ddmouse_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"

#include <functional>
#include <list>
#include <queue>
#include <windows.h>
namespace NSP_DD {
struct DDMOUSE_EVENT
{
    enum class DDMOUSE_EVENT_TYPE
    {
        NONE = 0,

        LDOWN,
        LUP,
        LDBCLICK,

        RDOWN,
        RUP,
        RDBCLICK,

        MDOWN,
        MUP,
        MDBCLICK,

        MID_WHELL,

        MOVE,
        ENTER,
        LEAVE,
    };

    DDMOUSE_EVENT_TYPE type = DDMOUSE_EVENT_TYPE::NONE;
    s32 relative_pos_x = 0;
    s32 relative_pos_y = 0;
    s32 wheel_del = 0;
    bool is_ldown = false;
    bool is_rdown = false;
    bool is_mdown = false;
};

using DDMOUSE_EVENT_CB = std::function<bool(const DDMOUSE_EVENT&)>;

class ddmouse
{
    DDNO_COPY_MOVE(ddmouse);
public:
    ddmouse() = default;
    ~ddmouse() = default;

    // 全局鼠标按键是否按下
    static bool is_key_down(int key);
    static bool is_ldown();
    static bool is_rdown();
    static bool is_mdown();

public:
    bool on_msg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    DDMOUSE_EVENT_CB ON_MOUSE;

private:
    bool m_captured = false;
    bool call_cbs(const DDMOUSE_EVENT& me);

private:
    bool try_leave(HWND hwnd, const DDMOUSE_EVENT& me);
    bool try_enter(HWND hwnd, const DDMOUSE_EVENT& me);
};

} // namespace NSP_DD
#endif // ddwin_ddmouse_h_