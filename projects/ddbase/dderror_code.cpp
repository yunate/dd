#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"
#include "ddbase/str/ddstr.h"
#include <tchar.h>
namespace NSP_DD {

#define _DDSTR(STR) #STR
#define _DDCONNECT(P1, P2) _DDSTR(P1 ## P2)

#define DDERROR_CODE_STRING_BEGIN(begin) { constexpr DWORD __count_base__ = __COUNTER__;  constexpr DWORD __begin__ = DWORD(dderror_code::begin);
#define DDERROR_CODE_STRING_END() }

#define DDERROR_CODE_STRING(NAME, STR)                                                                                                                              \
{                                                                                                                                                                   \
    static_assert(__COUNTER__ - __count_base__ - 1 + __begin__ == DWORD(dderror_code::NAME),                                                                        \
        _DDCONNECT(You have modify the enum dderror_code but the ddget_dd_error_msgw function was not synchronously modified. Error at dderror_code::, NAME));    \
    if (error_code == DWORD(dderror_code::NAME)) {                                                                                                                  \
        return STR;                                                                                                                                                 \
    }                                                                                                                                                               \
}

#define DDERROR_CODE_STRING1(NAME) DDERROR_CODE_STRING(NAME, _T(_DDCONNECT(dderror_code::, NAME)))

static std::wstring ddget_dd_error_msgw(DWORD error_code)
{
    DDERROR_CODE_STRING_BEGIN(ok)
        DDERROR_CODE_STRING1(ok);
        DDERROR_CODE_STRING1(unexpected);
        DDERROR_CODE_STRING1(out_of_memory);
        DDERROR_CODE_STRING1(time_out);
        DDERROR_CODE_STRING1(param_mismatch);
        DDERROR_CODE_STRING1(param_nullptr);
        DDERROR_CODE_STRING1(param_invalid_handle);
    DDERROR_CODE_STRING_END()

    DDERROR_CODE_STRING_BEGIN(init_failure)
        DDERROR_CODE_STRING1(init_failure);
        DDERROR_CODE_STRING1(init_repeat);
        DDERROR_CODE_STRING1(key_not_find);
        DDERROR_CODE_STRING1(key_exist);
        DDERROR_CODE_STRING1(buff_size_not_enough);
        DDERROR_CODE_STRING1(handle_closed);
        DDERROR_CODE_STRING1(operate_pending);
        DDERROR_CODE_STRING1(operate_repeat);
        DDERROR_CODE_STRING1(object_released);
        DDERROR_CODE_STRING1(inner_task_run);
        DDERROR_CODE_STRING1(invalid_url);
        DDERROR_CODE_STRING1(end);
    DDERROR_CODE_STRING_END()
    return L"unknown error.";
}

DWORD dderror_code::get_last_error()
{
    return ::GetLastError();
}

void dderror_code::set_last_error(DWORD error_code)
{
    ::SetLastError(error_code);
}

std::string dderror_code::get_error_msga(DWORD error_code)
{
    return ddstr::utf16_8(dderror_code::get_error_msgw(error_code));
}

std::string dderror_code::get_last_error_msga()
{
    return dderror_code::get_error_msga(dderror_code::get_last_error());
}

std::wstring dderror_code::get_last_error_msgw()
{
    return dderror_code::get_error_msgw(dderror_code::get_last_error());
}

std::wstring dderror_code::get_error_msgw(DWORD error_code)
{
    if ((error_code & (0x20000000)) != 0) {
        return ddget_dd_error_msgw(error_code);
    }

    HLOCAL buff = NULL;
    ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, 0, (LPWSTR)&buff, 0, NULL);
    std::wstring msg = (LPWSTR)buff;
    ::LocalFree(buff);
    return msg;
}

///////////////////////////////////////////dderror_code_guard///////////////////////////////////////////
dderror_code_guard::dderror_code_guard()
{
    m_error_code = dderror_code::get_last_error();
}

dderror_code_guard::~dderror_code_guard()
{
    dderror_code::set_last_error(m_error_code);
}

} // namespace NSP_DD

