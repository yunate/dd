
#include "ddtools/stdafx.h"

#include "ddbase/ddmini_include.h"
#include "ddbase/ddio.h"
#include "ddbase/ddtest_case_factory.h"


namespace NSP_DD {
DDTEST(project_packager, main)
{
    std::vector<std::wstring> include_file = { L"lib", L"dll", L"exe", L"pdb", L"h", L"hpp" };
    const std::wstring src_full_path = LR"__(F:\My\ddwork_space\dd)__";
    const std::wstring dst_full_path = LR"__(F:\My\ddwork_space\ddexport)__";
    std::wstring current_copy_path;
    if (dddir::copy_path_ex(src_full_path, dst_full_path, [&include_file, &current_copy_path](const std::wstring& full_path) {
        if (full_path.find(L"\\tmp\\") != std::wstring::npos) {
            return true;
        }
        if (full_path.find(L"\\__DD_DEMO__\\") != std::wstring::npos) {
            return true;
        }
        if (full_path.find(L"projects\\test") != std::wstring::npos) {
            return true;
        }
        //if (full_path.find(L"_.h") != std::wstring::npos) {
        //    return true;
        //}
        //if (full_path.find(L"_.hpp") != std::wstring::npos) {
        //    return true;
        //}
        std::wstring suffix = ddpath::suffix(full_path);
        for (const auto& it : include_file) {
            if (suffix == it) {
                current_copy_path = full_path;
                // ddcout(ddconsole_color::gray) << L"copy file: " << full_path << L"\r\n";
                return false;
            }
        }
        return true;
    })) {
        ddcout(ddconsole_color::green) << L"successful!\n";
    } else {
        ddcout(ddconsole_color::red) << L"copy file failure: " << current_copy_path  << L"\n";
        ddcout(ddconsole_color::red) << L"error: " << last_error_msgw(::GetLastError()) << L"\n";
    }
}
} // namespace NSP_DD

