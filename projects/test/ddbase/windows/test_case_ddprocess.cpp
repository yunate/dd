
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
    ddtimer timer;
    std::wstring target = LR"(\.vs\)";
    std::vector<ddhandle_info> infos;
    (void)ddprocess::enum_file_handles([&infos, &target](const ddhandle_info& info) {
        if (info.base_object_name.find(target) != std::wstring::npos) {
            infos.push_back(info);
        }
    });

    if (infos.empty()) {
        ddcout(ddconsole_color::cyan) << ddstr::format("cannot find. \n");
    } else {
        std::unordered_set<ULONG_PTR> processids;
        for (auto& it : infos) {
            if (processids.find(it.process_id) == processids.end()) {
                processids.insert(it.process_id);
                ddcout(ddconsole_color::cyan) << ddstr::format("processid: %d \n", it.process_id);
            }
        }
    }
    ddcout(ddconsole_color::cyan) << ddstr::format("test_case_ddprocess2 cost:%d \n", timer.get_time_pass() / 1000000);
}
} // namespace NSP_DD
