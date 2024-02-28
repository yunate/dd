#include "ddtools/stdafx.h"
#include "ddtools/sln_maker/sln_maker_helper.h"
#include "ddtools/sln_maker/solusion_maker.h"

#include "ddbase/str/ddstr.h"
#include "ddbase/file/ddfile.h"
#include "ddbase/file/dddir.h"
#include "ddbase/file/ddpath.h"
#include "ddbase/ddio.h"
#include "ddbase/ddrandom.h"


namespace NSP_DD {
static std::string project_type_string(ddproject_type type)
{
    if (type == ddproject_type::exe) {
        return "Application";
    }

    if (type == ddproject_type::lib) {
        return "StaticLibrary";
    }

    if (type == ddproject_type::dll) {
        return "DynamicLibrary";
    }
    return "";
}

ddsolusion_maker::ddsolusion_maker(const ddsolusion_maker_param& param)
    : m_param(param)
{
    ddpath::normal(m_param.base_dir);
    m_templete = L"../../projects/ddtools/sln_maker/templete/single";
}

bool ddsolusion_maker::make_base_dir()
{
    if (dddir::is_path_exist(m_param.base_dir)) {
        if (!dddir::is_dir(m_param.base_dir)) {
            ddcout(red) << ddstr::format(L"%s is a file.\n", m_param.base_dir.c_str());
            return false;
        }
    } else {
        if (!dddir::create_dir_ex(m_param.base_dir)) {
            ddcout(red) << ddstr::format(L"create dir %s failure.\n", m_param.base_dir.c_str());
            return false;
        }
    }

    std::wstring projects_dir = ddpath::join(m_param.base_dir, L"projects");
    if (dddir::is_path_exist(projects_dir)) {
        if (!dddir::is_dir(projects_dir)) {
            ddcout(red) << ddstr::format(L"%s is a file.\n", projects_dir.c_str());
            return false;
        }
    } else {
        if (!dddir::create_dir_ex(projects_dir)) {
            ddcout(red) << ddstr::format(L"create dir %s failure.\n", projects_dir.c_str());
            return false;
        }
    }

    return true;
}

bool ddsolusion_maker::copy_templete_and_replace()
{
    std::wstring proj_base_dir = ddpath::join({ m_param.base_dir, L"projects", m_param.proj_name});
    if (dddir::is_path_exist(proj_base_dir)) {
        ddcout(red) << ddstr::format(L"Gen proj_base_dir:[%s] has exist\n", proj_base_dir.c_str()).c_str();
        return false;
    }

    // MT/MD
    std::vector<std::string> defined_macro;
    if ((m_param.mt_d & (u32)DDMT_D::MD) != 0) {
        defined_macro.push_back("MD");
    }
    if ((m_param.mt_d & (u32)DDMT_D::MT) != 0) {
        defined_macro.push_back("MT");
    }
    if (m_param.use_vcpkg) {
        defined_macro.push_back("USE_VCPKG");
    }

    std::string proj_namea;
    ddstr::utf16_8(m_param.proj_name, proj_namea);
    if (!ddguid::generate_guid(m_proj_guid)) {
        ddcout(red) << L"Gen project_guid failure\n";
        return false;
    }

    std::vector<std::string> finder = { "__DD_DEMO__", "__PROJECT_GUID__", "__TYPE__" };
    std::vector<std::string> relplacer = { proj_namea, m_proj_guid, project_type_string(m_param.proj_type) };
    if (!ddsln_maker_helper::copy_and_replace(
        ddpath::join(m_templete, L"projects"),
        ddpath::join({ m_param.base_dir, L"projects" }),
        finder,
        relplacer,
        [&](const std::wstring& full_path) {
        if (!m_param.use_vcpkg) {
            if (ddstr::endwith(full_path.c_str(), L"vcpkg.json") || ddstr::endwith(full_path.c_str(), L"vcpkg-configuration.json")) {
                return false;
            }
        }
        return true;
    })) {
        ddcout(red) << L"copy_and_replace project failure\n";
        return false;
    }

    bool is_successful = true;
    dddir::enum_dir(proj_base_dir, [&is_successful, &defined_macro](const std::wstring& full_path, bool isDir) {
        if (!isDir) {
            if (!ddsln_maker_helper::expand_macro(full_path, defined_macro)) {
                ddcout(red) << L"expand_macro failure\n";
                is_successful = false;
                return true;
            }
        }
        return false;
    });

    if (!is_successful) {
        return false;
    }

    return true;
}

bool ddsolusion_maker::make_sln()
{
    if (m_param.sln_name.empty()) {
        return true;
    }

    std::wstring full_sln_path = ddpath::join(m_param.base_dir, m_param.sln_name) + L".sln";
    if (dddir::is_path_exist(full_sln_path)) {
        if (dddir::is_dir(full_sln_path)) {
            ddcout(red) << ddstr::format(L"%s is a dir.\n", full_sln_path.c_str());
            return false;
        }

        return true;
    }

    std::string sln_guid;
    if (!ddguid::generate_guid(sln_guid)) {
        ddcout(red) << L"Gen sln_guid failure\n";
        return false;
    }

    std::wstring full_sln_templete_path = ddpath::join(m_templete, L"templete.sln");
    std::vector<std::tuple<std::string, std::string>> replacer {
        {"__SLN_GUID__", sln_guid},
    };
    std::vector<std::string> finder = { "__SLN_GUID__" };
    std::vector<std::string> relplacer = { sln_guid };
    if (!ddsln_maker_helper::copy_and_replace(full_sln_templete_path, full_sln_path, finder, relplacer)) {
        ddcout(red) << L"copy_and_replace sln failure\n";
        return false;
    }

    return true;
}

bool ddsolusion_maker::add_proj_2_sln()
{
    if (m_param.sln_name.empty()) {
        return true;
    }

    std::wstring full_sln_path = ddpath::join(m_param.base_dir, m_param.sln_name) + L".sln";
    if (!dddir::is_path_exist(full_sln_path)) {
        ddcout(red) << ddstr::format(L"%s is not exit.\n", full_sln_path.c_str());
        return false;
    }

    if (dddir::is_dir(full_sln_path)) {
        ddcout(red) << ddstr::format(L"%s is a dir.\n", full_sln_path.c_str());
        return false;
    }

    std::vector<std::string> lines;
    {
        std::shared_ptr<ddfile> reader(ddfile::create_utf8_file(full_sln_path));
        if (reader == nullptr) {
            ddcout(red) << ddstr::format(L"read sln file %s failure.\n", full_sln_path.c_str());
            return false;
        }

        std::string line;
        while (reader->read_linea(line)) {
            lines.push_back(line);
            line.clear();
        }
    }

    std::shared_ptr<ddfile> writer(ddfile::create_utf8_file(full_sln_path));
    if (writer == nullptr) {
        ddcout(red) << ddstr::format(L"open file %s to write failure.\n", full_sln_path.c_str());
        return false;
    }

    std::string guid = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}";

    for (const auto& line : lines) {
        if ((s32)line.size() != writer->write((const u8*)line.c_str(), (u32)line.size())) {
            ddcout(red) << ddstr::format(L"write generated code to %s failure.\n", full_sln_path.c_str());
            return false;
        }

        if (line.find("MinimumVisualStudioVersion") != std::string::npos) {
            // Project("guid") = "proj_name", "proj_name", "proj_guid"
            // EndProject
            std::string proj_name;
            ddstr::utf16_8(m_param.proj_name, proj_name);
            std::string temp = R"__(Project("%s") = "%s", "projects\%s\%s.vcxproj", "%s"
EndProject
)__";
            temp = ddstr::format(temp.c_str(), guid.c_str(), proj_name.c_str(), proj_name.c_str(), proj_name.c_str(), m_proj_guid.c_str());
            if ((s32)temp.size() != writer->write((const u8*)temp.c_str(), (u32)temp.size())) {
                ddcout(red) << ddstr::format(L"write generated code to %s failure.\n", full_sln_path.c_str());
                return false;
            }
        }

        if (line.find("GlobalSection(ProjectConfigurationPlatforms)") != std::string::npos) {
            // proj_guid.Debug_Mdd|x64.ActiveCfg = Debug_Mdd|x64
            // proj_guid.Debug_Mdd|x64.Build.0 = Debug_Mdd|x64
            // proj_guid.Debug_Mdd|x86.ActiveCfg = Debug_Mdd|Win32
            // proj_guid.Debug_Mdd|x86.Build.0 = Debug_Mdd|Win32
            // proj_guid.Debug|x64.ActiveCfg = Debug|x64
            // proj_guid.Debug|x64.Build.0 = Debug|x64
            // proj_guid.Debug|x86.ActiveCfg = Debug|Win32
            // proj_guid.Debug|x86.Build.0 = Debug|Win32
            // proj_guid.Release_Md|x64.ActiveCfg = Release_Md|x64
            // proj_guid.Release_Md|x64.Build.0 = Release_Md|x64
            // proj_guid.Release_Md|x86.ActiveCfg = Release_Md|Win32
            // proj_guid.Release_Md|x86.Build.0 = Release_Md|Win32
            // proj_guid.Release|x64.ActiveCfg = Release|x64
            // proj_guid.Release|x64.Build.0 = Release|x64
            // proj_guid.Release|x86.ActiveCfg = Release|Win32
            // proj_guid.Release|x86.Build.0 = Release|Win32
            std::vector<std::string> tmps;
            std::vector<std::string> tmp_mdd = {
                "\t\t%s.Debug_Mdd|x64.ActiveCfg = Debug_Mdd|x64\r\n",
                "\t\t%s.Debug_Mdd|x64.Build.0 = Debug_Mdd|x64\r\n",
                "\t\t%s.Debug_Mdd|x86.ActiveCfg = Debug_Mdd|Win32\r\n",
                "\t\t%s.Debug_Mdd|x86.Build.0 = Debug_Mdd|Win32\r\n",
            };
            std::vector<std::string> tmp_md = {
                "\t\t%s.Release_Md|x64.ActiveCfg = Release_Md|x64\r\n",
                "\t\t%s.Release_Md|x64.Build.0 = Release_Md|x64\r\n",
                "\t\t%s.Release_Md|x86.ActiveCfg = Release_Md|Win32\r\n",
                "\t\t%s.Release_Md|x86.Build.0 = Release_Md|Win32\r\n",
            };
            std::vector<std::string> tmp_mtd = {
                "\t\t%s.Debug|x64.ActiveCfg = Debug|x64\r\n",
                "\t\t%s.Debug|x64.Build.0 = Debug|x64\r\n",
                "\t\t%s.Debug|x86.ActiveCfg = Debug|Win32\r\n",
                "\t\t%s.Debug|x86.Build.0 = Debug|Win32\r\n",
            };
            std::vector<std::string> tmp_mt = {
                "\t\t%s.Release|x64.ActiveCfg = Release|x64\r\n",
                "\t\t%s.Release|x64.Build.0 = Release|x64\r\n",
                "\t\t%s.Release|x86.ActiveCfg = Release|Win32\r\n",
                "\t\t%s.Release|x86.Build.0 = Release|Win32\r\n",
            };
            if ((m_param.mt_d & (u32)DDMT_D::MD) != 0) {
                std::copy(tmp_mdd.begin(), tmp_mdd.end(), std::back_inserter(tmps));
            }
            if ((m_param.mt_d & (u32)DDMT_D::MT) != 0) {
                std::copy(tmp_mtd.begin(), tmp_mtd.end(), std::back_inserter(tmps));
            }
            if ((m_param.mt_d & (u32)DDMT_D::MD) != 0) {
                std::copy(tmp_md.begin(), tmp_md.end(), std::back_inserter(tmps));
            }
            if ((m_param.mt_d & (u32)DDMT_D::MT) != 0) {
                std::copy(tmp_mt.begin(), tmp_mt.end(), std::back_inserter(tmps));
            }

            for (const auto& it : tmps) {
                std::string tmp = ddstr::format(it.c_str(), m_proj_guid.c_str());
                if ((s32)tmp.size() != writer->write((u8*)tmp.c_str(), (u32)tmp.size())) {
                    ddcout(red) << ddstr::format(L"write generated code to %s failure.\n", full_sln_path.c_str());
                    return false;
                }
            }
        }
    }
    return true;
}

bool ddsolusion_maker::copy_user_props()
{
    std::wstring full_path = ddpath::join(m_param.base_dir, L"user.props");
    if (dddir::is_path_exist(full_path)) {
        if (dddir::is_dir(full_path)) {
            ddcout(red) << ddstr::format(L"%s is a dir.\n", full_path.c_str());
            return false;
        }

        return true;
    }

    std::wstring templete_path = ddpath::join(m_templete, L"user.props");
    return dddir::copy_path(templete_path, full_path);
}

bool ddsolusion_maker::make()
{
    if (!make_base_dir()) {
        ddcout(red) << L"make_base_dir failure\n";
        return false;
    }

    if (!copy_templete_and_replace()) {
        ddcout(red) << L"copy_templete_and_replace failure\n";
        return false;
    }

    if (!make_sln()) {
        ddcout(red) << L"make_sln failure\n";
        return false;
    }

    if (!add_proj_2_sln()) {
        ddcout(red) << L"add_proj_2_sln failure\n";
        return false;
    }

    if (!copy_user_props()) {
        ddcout(red) << L"copy_user_props failure\n";
        return false;
    }

    ddcout(green) << L"make successful\n";
    return true;
}

} // namespace NSP_DD
