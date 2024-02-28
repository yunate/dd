#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddpra/ddmouse_simulater.h"

namespace NSP_DD {

DDTEST(test_case_mouse_move, 1)
{
    u32 x = 500;
    u32 y = 500;
    for (int i = 0; i < 1000; ++i) {
        ddmouse_simulater::move_to(x, y, 0);
        x += 10;
        y += 10;
        ::Sleep(100);
    }
}

} // namespace NSP_DD
