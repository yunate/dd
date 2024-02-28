#include "ddbase/stdafx.h"
#include "ddbase/ddcmd_line_utils.h"
#include "ddbase/str/ddstr.h"
#include <windows.h>
#include <shellapi.h>
namespace NSP_DD {

static void handle_space(std::wstring& cmd_str)
{
    size_t index = 0;
    bool l_quotes = false;
    for (size_t i = 0; i < cmd_str.size(); ++i) {
        auto c = cmd_str[i];
        if (c == L'"') {
            l_quotes = !l_quotes;
        }

        if (c == ' ') {
            if (!l_quotes) {
                // 双引号之外的空格替换成1
                c = 1;
                if (index > 0 && cmd_str[index - 1] == 1) {
                    // 跳过连续的空格
                    continue;
                }
            }
        }

        cmd_str[index++] = c;
    }

    cmd_str.resize(index);
}

void ddcmd_line_utils::get_cmds(std::vector<std::wstring>& cmds)
{
    cmds.clear();
    std::wstring cmd_str = ::GetCommandLineW();
    handle_space(cmd_str);
    std::wstring spliter(1, 1);
    ddstr::split(cmd_str.c_str(), spliter.c_str(), cmds);
    for (auto& it : cmds) {
        ddstr::trim(it, L'"');
    }
}

} // namespace NSP_DD

