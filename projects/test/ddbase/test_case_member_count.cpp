
#include "test/stdafx.h"
#include "ddbase/ddmember_count.hpp"
#include "ddbase/ddassert.h"
#include "ddbase/ddtest_case_factory.h"
#include <iostream>

namespace NSP_DD {
DDTEST(test_member_count, member_count1)
{
    struct Test { int a; int b; int c; int d; };
    constexpr size_t x = ddmember_count_v<Test>;
    DDASSERT(x == 4);

    // constexpr size_t xx = ddmember_count_v<std::set<int>>;
}
} // namespace NSP_DD
