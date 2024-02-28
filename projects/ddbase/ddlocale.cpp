#include "ddbase/stdafx.h"
#include "ddbase/ddlocale.h"

#include <consoleapi.h>

namespace NSP_DD {
    bool ddlocale::set_utf8_locale_and_io_codepage()
    {
        auto raw_codepage = get_io_codepage();
        if (!set_io_codepage(65001)) {
            return false;
        }
        if (set_current_locale(".UTF-8") == NULL) {
            (void)set_io_codepage(raw_codepage);
            return false;
        }
        return true;
    }

    u32 ddlocale::get_current_locale_codepage_id()
    {
        u32 code_page = (s32)CP_ACP;
        ::_locale_t local = ::_get_current_locale();
        if (local != nullptr) {
            code_page = (s32)((::__crt_locale_data_public*)(local->locinfo))->_locale_lc_codepage;
            ::_free_locale(local);
        }
        return code_page;
    }

    bool ddlocale::set_current_locale(const std::string& name)
    {
        int config = ::_configthreadlocale(_DISABLE_PER_THREAD_LOCALE);
        if (config == -1) {
            return false;
        }

        if (::setlocale(LC_ALL, name.c_str()) == NULL) {
            return false;
        }
        (void)::_configthreadlocale(config);
        return true;
    }

    s32 ddlocale::get_io_codepage()
    {
        return s32(::GetConsoleOutputCP());
    }

    bool ddlocale::set_io_codepage(s32 code_page)
    {
        if (!::SetConsoleOutputCP(code_page)) {
            return false;
        }
        return true;
    }

    s32 ddlocale::get_system_codepage()
    {
        return s32(::GetACP());
    }
} // namespace NSP_DD

