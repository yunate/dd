#ifndef ddpra_ddglobal_info_h_
#define ddpra_ddglobal_info_h_
#include "ddbase/dddef.h"

namespace NSP_DD {
struct ddscreen_info
{
    bool active = false;
    bool primary = false;
    u32 index = 0;
    s32 offset_x = 0;
    s32 offset_y = 0;
    u32 screen_width = 0;
    u32 screen_height = 0;
    float w_65535 = 0.0f; // screen_width / 65535.0f
    float h_65535 = 0.0f; // screen_height / 65535.0f
};

struct ddglobal_info
{
    std::vector<ddscreen_info> screen_infos;
};

const ddglobal_info& get_ddglobal_info();
} // namespace NSP_DD
#endif // ddpra_ddmouse_simulater_h_
