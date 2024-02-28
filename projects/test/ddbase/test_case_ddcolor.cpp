#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddcolor.h"

namespace NSP_DD {
DDTEST(test_case_ddcolor, 1)
{
    {
        ddbgra _1 = { 1, 2, 3, 4 };
        ddbgra _2 = { 1, 2, 3, 4 };
        if (!(_1 == _2)) {
            DDASSERT(false);
        }
    }

    {
        ddargb _1 = { 1, 2, 3, 4 };
        ddargb _2 = { 1, 2, 3, 4 };
        if (!(_1 == _2)) {
            DDASSERT(false);
        }
    }

    {
        ddrgb _1 = { 1, 2, 3 };
        ddrgb _2 = { 1, 2, 3 };
        if (!(_1 == _2)) {
            DDASSERT(false);
        }
    }
}
} // namespace NSP_DD
