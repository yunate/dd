#include "ddtools/stdafx.h"
#include "ddtools/code_format/ddcode_snapshot_cpp.h"

#include "ddbase/ddmini_include.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddio.h"

namespace NSP_DD {
static void check_define(const std::vector<ddcode_snapshot_cpp_line_desc*>& descs, const std::wstring& parent_path, const std::wstring& sub_path, const std::wstring& name)
{
    std::string define = ddstr::utf16_8(ddpath::join(sub_path, name));
    define = ddstr::replace(define.c_str(), "/", "_");
    define = ddstr::replace(define.c_str(), "\\", "_");
    define = ddstr::replace(define.c_str(), ".", "_");
    define += "_";

    std::string first_line = "#ifndef " + define;
    std::string second_line = "#define " + define;
    std::string end_line = "#endif // " + define;

    do {
        size_t l = 0;
        size_t r = 0;
        for (size_t i = 0; i < descs.size(); ++i) {
            if (!descs[i]->pretty_str.empty()) {
                l = i;
                break;
            }
        }

        for (size_t i = descs.size(); i > 0 ; --i) {
            if (!descs[i - 1]->pretty_str.empty()) {
                r = i - 1;
                break;
            }
        }

        if (r - l + 1 < 3) {
            break;
        }

        std::string tmp = descs[l]->raw_str;
        ddstr::trim(tmp, std::vector<char>{ '\r', '\n', ' '});

        if (tmp != first_line) {
            break;
        }

        tmp = descs[l + 1]->raw_str;;
        ddstr::trim(tmp, std::vector<char>{ '\r', '\n', ' ' });
        if (tmp != second_line) {
            break;
        }

        tmp = descs[r]->raw_str;;
        ddstr::trim(tmp, std::vector<char>{ '\r', '\n', ' ' });
        if (tmp != end_line) {
            break;
        }
        return;
    } while (0);

    std::wstring normal_path = ddpath::join({ parent_path, sub_path, name });
    ddpath::normal(normal_path, L'/');
    ddcout(ddconsole_color::gray) << normal_path << "\r\n";
    ddcout(ddconsole_color::red) << L"the head file should begin with:\n";
    ddcout(ddconsole_color::red) << first_line << "\r\n";
    ddcout(ddconsole_color::red) << second_line << "\r\n";
    ddcout(ddconsole_color::red) << L"and should end with:\n";
    ddcout(ddconsole_color::red) << end_line << "\r\n";
    ddcout(ddconsole_color::red) <<"\r\n";
}

static bool should_skip(const std::wstring& sub_path)
{
    std::wstring normal_sub_path = ddpath::normal1(sub_path);
    if (normal_sub_path.find(ddpath::normal1(L"ddtools/sln_maker/templete")) != std::wstring::npos) {
        return true;
    }

    if (normal_sub_path.find(ddpath::normal1(L"old_version")) != std::wstring::npos) {
        return true;
    }

    return false;
}

void format(const std::wstring& root_dir_full_path)
{
    if (!dddir::is_dir(root_dir_full_path)) {
        ddcout(ddconsole_color::red) << L"the second param must be a directory full path. \n";
        return;
    }

    std::map<std::wstring, std::wstring> file_paths;
    dddir::enum_dir(root_dir_full_path, [&file_paths](const std::wstring& sub_path, const std::wstring& name, bool is_dir) {
        if (is_dir) {
            // skip the dir
            return false;
        }

        std::wstring normal_path = sub_path;
        ddpath::normal(normal_path, L'/');
        file_paths[name] = normal_path;
        return false;
    });

    dddir::enum_dir(root_dir_full_path, [&file_paths, &root_dir_full_path](const std::wstring& sub_path, const std::wstring& name, bool is_dir) {
        if (is_dir) {
            // skip the dir
            return false;
        }

        if (should_skip(sub_path)) {
            return false;
        }

        std::wstring full_path = ddpath::join({ root_dir_full_path, sub_path, name });
        std::wstring suffix = ddpath::suffix(full_path);
        if (suffix != L"h" && suffix != L"hpp") {
            return false;
        }

        ddcode_snapshot_cpp_file snapshot;
        if (!snapshot.analyze_file(full_path)) {
            return false;
        }

        const std::vector<ddcode_snapshot_cpp_line_desc*>& descs = snapshot.get_desc();
        check_define(descs, root_dir_full_path, sub_path, name);
        return false;
    });
}

DDTEST(code_format, format)
{
    format(L"../../projects/");
}
} // namespace NSP_DD
