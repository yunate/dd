#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddlocale.h"
#include <process.h>

#pragma comment(lib, "ddbase.lib")
#pragma comment(lib, "ddwin.lib")
#pragma comment(lib, "ddhook.lib")
#pragma comment(lib, "ddpra.lib")

namespace NSP_DD {

int test_main()
{
    DDTCF.insert_white_type("test_case_ddprocess2");

    // DDTCF.insert_white_type("test_dddir4");
    // DDTCF.insert_white_type("test_case_coroutine");
    // DDTCF.insert_white_type("test_case_coroutine1");

    // DDTCF.insert_white_type("test_case_http_server");
    // DDTCF.insert_white_type("test_case_http_requester");

    // DDTCF.insert_white_type("test_case_tcp_client");
    // DDTCF.insert_white_type("test_case_tcp_server");

    // DDTCF.insert_white_type("test_case_sync_pipe_client");
    // DDTCF.insert_white_type("test_case_sync_pipe_server");

    //  DDTCF.insert_white_type("test_case_co_ddiocp_with_dispatcher_client");
    //  DDTCF.insert_white_type("test_case_co_ddiocp_with_dispatcher_server");

    // DDTCF.insert_white_type("test_case_ddiocp_with_dispatcher_client");
    // DDTCF.insert_white_type("test_case_ddiocp_with_dispatcher_server");
    DDTCF.run();
    return 0;
}

} // namespace NSP_DD


void main()
{
    // ::_CrtSetBreakAlloc(918);

    NSP_DD::ddlocale::set_utf8_locale_and_io_codepage();
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

