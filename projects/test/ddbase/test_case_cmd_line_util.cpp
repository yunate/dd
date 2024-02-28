#include "test/stdafx.h"

#include "ddbase/ddcmd_line_utils.h"

#include "ddbase/ddtest_case_factory.h"

#include <iostream>

namespace NSP_DD {
DDTEST(test_cmd_line_util, get_cmds)
{
    std::vector<std::wstring> cmds;
    ddcmd_line_utils::get_cmds(cmds);
}
} // namespace NSP_DD
