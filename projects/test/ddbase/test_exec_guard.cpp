#include "test/stdafx.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/str/ddfast_str.hpp"

namespace NSP_DD {

DDTEST(test_ddexec_guard, test)
{
    bool flag = false;
    {
        ddexec_guard guard([&flag]() {
            flag = true;
        });
    }
    DDASSERT(flag);
}

} // namespace NSP_DD
