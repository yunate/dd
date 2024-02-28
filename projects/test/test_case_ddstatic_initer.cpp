#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddstatic_initer.h"


namespace NSP_DD {
DDSTATIC_INITER(test_ddstatic_init)
{
}

DDSTATIC_DEINITER(test_ddstatic_init)
{
}

DDSTATIC_INIT(test_ddstatic_init);

DDTEST(test_ddstatic_init, main)
{
}

} // namespace NSP_DD
