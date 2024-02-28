#include "ddtools/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddcmd_line_utils.h"
#include "ddbase/ddio.h"
#include <process.h>
#include "ddbase/ddlocale.h"

#pragma comment(lib, "ddbase.lib")

namespace NSP_DD {
int ddmain()
{
    NSP_DD::ddlocale::set_utf8_locale_and_io_codepage();

    // DDTCF.insert_white_type("searcher");
    // DDTCF.insert_white_type("code_format");

    DDTCF.insert_white_type("sln_maker");
    // DDTCF.insert_white_type("project_file_adder");
    // DDTCF.insert_white_type("project_packager");

    DDTCF.run();
    return 0;
}
} // namespace NSP_DD

void main()
{
    // ::_CrtSetBreakAlloc(918);

    int result = NSP_DD::ddmain();


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

