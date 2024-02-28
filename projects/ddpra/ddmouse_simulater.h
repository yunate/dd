#ifndef ddpra_ddmouse_simulater_h_
#define ddpra_ddmouse_simulater_h_
#include "ddbase/dddef.h"

namespace NSP_DD {
class ddmouse_simulater
{
public:
    static bool move_to(u32 x, u32 y, u32 screen_index = 0);
    static bool click_l_burron(u32 x, u32 y, u32 screen_index = 0);
    static bool click_r_burron(u32 x, u32 y, u32 screen_index = 0);
    static bool click_m_burron(u32 x, u32 y, u32 screen_index = 0);
    static bool double_click_l_burron(u32 x, u32 y, u32 screen_index = 0);
    static bool double_click_r_burron(u32 x, u32 y, u32 screen_index = 0);
};
} // namespace NSP_DD
#endif // ddpra_ddmouse_simulater_h_
