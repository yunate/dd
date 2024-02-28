
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/ddprocess.h"
#include "ddbase/ddio.h"
#include "ddbase/str/ddstr.h"

namespace NSP_DD {

DDTEST(test_process, get_process_id)
{
    ddprocess p1;
    (void)p1.init(std::wstring(L"notepad.exe"));

    ddprocess p2;
    (void)p2.init(11104);

    ddprocess p3;
    HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, 11104);
    if (hProcess != NULL) {
        (void)p3.init(hProcess, 0);
    }
}

DDTEST(test_case_ddprocess1, 1)
{
    ddprocess::enum_process([](const ddprocess_info& info) {
        ddcout(ddconsole_color::gray) << ddstr::format(L"name: %s; fullPath: %s; id: %d; parent_id: %d; x64: %s;\r\n",
            info.name.c_str(), info.fullPath.c_str(), info.id, info.parent_id, info.x64 ? L"true" : L"false");
        return true;
    });
}

DDTEST(test_case_ddprocess2, 1)
{
    std::vector<ddhandle_info> infos;
    ddprocess::enum_all_handles([&infos](const ddhandle_info& info) {
        // if (ddstr::lower(info.type_name.c_str()) != L"file") {
        //     return;
        // }

        if (ddstr::lower(info.base_object_name.c_str()) != ddstr::lower(LR"(C:\Program Files (x86)\Microsoft\EdgeWebView\Application\138.0.3351.121)")) {
            return;
        }
        infos.push_back(info);
    });
}
} // namespace NSP_DD
