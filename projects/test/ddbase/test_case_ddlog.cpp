#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddlog.hpp"


namespace NSP_DD {
DDTEST(test_ddlog, get_time_pass)
{
    DDLOG(ERROR, "abcdefg");
    DDLOGW(ERROR, L"abcdefg");
    DDLOG1(ERROR, "abcdefg");
    DDLOG1W(ERROR, L"abcdefg");
}

} // namespace NSP_DD
