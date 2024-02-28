
#include "test/stdafx.h"
#include "ddbase/file/dddir_spyer.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddio.h"
#include "ddbase/str/ddstr.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/ddmoudle_utils.h"
#include <iterator>

namespace NSP_DD {
static std::wstring get_spy_file()
{
    // return ddpath::parent(ddmoudle_utils::get_moudle_pathW());
    ddcout(ddconsole_color::gray) << L"please input user_data_dir full path \r\n";
    std::wstring dir;
    ddcin() >> dir;
    return dir;
}

static bool filter(const std::wstring& sub_path)
{
    if (ddstr::lower(sub_path.c_str()) != ddstr::lower(L"Local State")) {
        return false;
    }

    return true;
}

DDTEST(test_case_dddir_spyer, ddfile)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    auto spyer = dddir_spyer::create_inst(iocp.get());
    if (spyer == nullptr) {
        return;
    }

    s32 count = 0;
    auto piocp = iocp.get();
    auto spy_dir_path = get_spy_file();
    if (!spyer->spy(spy_dir_path, [piocp, spy_dir_path, &count](const std::wstring& sub_path, dddir_spyer::type type, const dddir_spyer::continue_spy& spy) {
        if (filter(sub_path)) {
            ddcout(ddconsole_color::green) << ddstr::format(L"%d %s", ++count, ddpath::join(spy_dir_path, sub_path).c_str());
            if (type == dddir_spyer::type::added) {
                ddcout(ddconsole_color::green) << L" added.\r\n";
            } else if (type == dddir_spyer::type::removed) {
                ddcout(ddconsole_color::green) << L" removed.\r\n";
            } else if (type == dddir_spyer::type::modified) {
                ddcout(ddconsole_color::green) << L" modified.\r\n";
            } else if (type == dddir_spyer::type::rename) {
                ddcout(ddconsole_color::green) << L" rename.\r\n";
            } else if (type == dddir_spyer::type::error) {
                ddcout(ddconsole_color::red) << L" error.\r\n";
            }
        }
        if (!spy()) {
            piocp->notify_close();
        }
    })) {
        ddcout(ddconsole_color::red) << L"spy " << spy_dir_path << " error:" << dderror_code::get_last_error_msgw() << L"\r\n";
        piocp->notify_close();
    } else {
        ddcout(ddconsole_color::green) << spy_dir_path << " spying...\r\n";
    }

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
}

} // namespace NSP_DD
