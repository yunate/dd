#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/ddmoudle_utils.h"


namespace NSP_DD {
DDTEST(test_moudle_utils, get_moudle)
{
    HMODULE moudleA = ddmoudle_utils::get_moudleA(NULL);
    HMODULE moudleA1 = ddmoudle_utils::get_moudleA("");
    HMODULE moudleW = ddmoudle_utils::get_moudleW(NULL);
    HMODULE moudleW1 = ddmoudle_utils::get_moudleW(L"");
    DDASSERT(moudleW == moudleA);
    DDASSERT(moudleW1 == moudleA1);

    std::string full_path = ddmoudle_utils::get_moudle_pathA(moudleA);
}

} // namespace NSP_DD
