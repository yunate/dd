#include "test/stdafx.h"

#include "ddbase/ddlocale.h"

namespace NSP_DD {

    DDTEST(test_ddlocale, main)
    {
        s32 sys_codepage_id = ddlocale::get_system_codepage();
        s32 io_codepage_id = ddlocale::get_io_codepage();
        // io 默认编码是和系统一致的
        DDASSERT(sys_codepage_id == io_codepage_id);

        s32 codepage_id = ddlocale::get_current_locale_codepage_id();
        // 当前locale的编码为0
        DDASSERT(codepage_id == 0);

        (void)ddlocale::set_utf8_locale_and_io_codepage();
        s32 codepage_id1 = ddlocale::get_current_locale_codepage_id();
        s32 io_codepage_id1 = ddlocale::get_io_codepage();
        DDASSERT(codepage_id1 == io_codepage_id1);
        DDASSERT(codepage_id1 == 65001);
    }
} // namespace NSP_DD
