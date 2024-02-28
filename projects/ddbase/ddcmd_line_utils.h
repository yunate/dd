#ifndef ddbase_ddcmd_line_utils_h_
#define ddbase_ddcmd_line_utils_h_

#include "ddbase/dddef.h"
#include <string>

namespace NSP_DD {
class ddcmd_line_utils
{
public:
    static void get_cmds(std::vector<std::wstring>& cmds);
};
} // namespace NSP_DD
#endif // ddbase_ddcmd_line_utils_h_
