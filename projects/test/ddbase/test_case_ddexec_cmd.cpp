#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/ddprocess.h"

namespace NSP_DD {
DDTEST(test_case_ddexec_cmd, exec)
{
    std::string result;
    DDASSERT(ddsub_process::exec_cmd("dir", result));
    DDASSERT(ddsub_process::exec_cmd("git st", result));

    auto inst = ddsub_process::create_instance("", "cmd");
    inst->write("git st\n");
    inst->read(result);
}

} // namespace NSP_DD
