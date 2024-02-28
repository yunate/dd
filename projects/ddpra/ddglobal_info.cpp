#include "ddpra/stdafx.h"
#include "ddpra//ddglobal_info.h"
#include <windows.h>
#include <mutex>

namespace NSP_DD {
static void init_screen_info(ddglobal_info& gi)
{
    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICE);
    int index = 0;
    while (::EnumDisplayDevices(NULL, index, &displayDevice, 0)) {
        ddscreen_info si;
        si.active = displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE;
        si.primary = displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE;
        si.index = index;
        DEVMODE dev_mode = { 0 };
        dev_mode.dmSize = sizeof(DEVMODE);
        if (::EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dev_mode)) {
            si.offset_x = dev_mode.dmPosition.x;
            si.offset_y = dev_mode.dmPosition.y;
            si.screen_width = dev_mode.dmPelsWidth;
            si.screen_height = dev_mode.dmPelsHeight;
            si.w_65535 = 65535.0f / si.screen_width;
            si.h_65535 = 65535.0f / si.screen_height;
            if (si.screen_height > 0 && si.screen_width > 0) {
                gi.screen_infos.push_back(si);
            }
        }
        ++index;
    }
}

static void init_global_info(ddglobal_info& gi)
{
    init_screen_info(gi);
}

const ddglobal_info& get_ddglobal_info()
{
    static ddglobal_info gi;
    static bool inited = false;
    if (!inited) {
        inited = true;
        init_global_info(gi);
    }
    return gi;
}
} // namespace NSP_DD
