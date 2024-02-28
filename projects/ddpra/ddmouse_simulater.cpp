#include "ddpra/stdafx.h"
#include "ddpra/ddmouse_simulater.h"
#include "ddpra/ddglobal_info.h"
#include <windows.h>

namespace NSP_DD {

bool ddmouse_simulater::move_to(u32 x, u32 y, u32 screen_index)
{ 
    const auto& gi = get_ddglobal_info();
    if (screen_index >= gi.screen_infos.size()) {
        return false;
    }
    const auto& si = gi.screen_infos[screen_index];
    if (x > si.screen_width || y > si.screen_height) {
        return false;
    }
    return !!::SetCursorPos((x + si.offset_x), (y + si.offset_y));
}

bool ddmouse_simulater::click_l_burron(u32 x, u32 y, u32 screen_index)
{
    x, y, screen_index;
    return false;

}

bool ddmouse_simulater::click_r_burron(u32 x, u32 y, u32 screen_index)
{
    x, y, screen_index;
    return false;

}

bool ddmouse_simulater::click_m_burron(u32 x, u32 y, u32 screen_index)
{
    x, y, screen_index;
    return false;

}

bool ddmouse_simulater::double_click_l_burron(u32 x, u32 y, u32 screen_index)
{
    x, y, screen_index;
    return false;

}

bool ddmouse_simulater::double_click_r_burron(u32 x, u32 y, u32 screen_index)
{
    x, y, screen_index;
    return false;
}

} // namespace NSP_DD
