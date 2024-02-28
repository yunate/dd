#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include <process.h>

#pragma comment(lib, "ddbase.lib")
#pragma comment(lib, "ddwin.lib")
#pragma comment(lib, "ddhook.lib")

namespace NSP_DD {

int test_main()
{
    DDTCF.insert_white_type("test_case_co_ddiocp_with_dispatcher_client");
    DDTCF.run();
    return 0;
}

} // namespace NSP_DD


void main()
{
    // ::_CrtSetBreakAlloc(918);

    int result = NSP_DD::test_main();

#ifdef _DEBUG
    _cexit();
    DDASSERT_FMT(!::_CrtDumpMemoryLeaks(), L"Memory leak!!! Check the output to find the log.");
    ::system("pause");
    ::_exit(result);
#else
    ::system("pause");
    ::exit(result);
#endif
}

