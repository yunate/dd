
#include "test/stdafx.h"
#include "ddbase/ddmember_count.hpp"
#include "ddbase/ddassert.h"
#include "ddbase/ddtest_case_factory.h"
#include <iostream>

namespace NSP_DD {
DDTEST(test_member_count, member_count1)
{
    struct Test { int a; int b; int c; int d; };
    int x = ddmember_count<Test>::value;
    DDASSERT(x == 4);
}
} // namespace NSP_DD
