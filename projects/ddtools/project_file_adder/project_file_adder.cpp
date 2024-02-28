#include "ddtools/stdafx.h"
#include "ddtools/project_file_adder/project_file_adder.h"
#include "ddbase/ddio.h"

namespace NSP_DD {
project_file_adder::project_file_adder(const std::wstring& vcxproj_full_path, const std::wstring& src_full_path, const std::wstring& filter_prefix)
{
    m_filter_prefix = ddpath::normal1(filter_prefix);
    m_vcxproj_full_path = ddpath::expand1(vcxproj_full_path);
    m_vcxproj_parent_path = ddpath::parent(m_vcxproj_full_path);
    m_src_full_path = ddpath::expand1(src_full_path);
    m_src_parent_path = ddpath::parent(src_full_path);
}

static bool is_head_file(const std::wstring& suffix)
{
    return suffix == L"h" || suffix == L"hpp";
}

static bool is_c_file(const std::wstring& suffix)
{
    return suffix == L"c" || suffix == L"cpp";
}

static bool is_should_handle_file(const std::wstring& suffix)
{
    return is_head_file(suffix) || is_c_file(suffix);
}

bool project_file_adder::add()
{
    std::shared_ptr<ddfile> vcxproj_file(ddfile::create_utf8_file(m_vcxproj_full_path));
    if (vcxproj_file == nullptr) {
        ddcout(ddconsole_color::red) << L"vcxproj file not found: " << m_vcxproj_full_path << L"\n";
        return false;
    }

    std::wstring vcxproj_filters_path = m_vcxproj_full_path + L".filters";
    std::shared_ptr<ddfile> vcxproj_filters_file(ddfile::create_utf8_file(vcxproj_filters_path));
    if (vcxproj_filters_file == nullptr) {
        ddcout(ddconsole_color::red) << L"vcxproj.filters file not found: " << vcxproj_filters_path << L"\n";
        return false;
    }

    if (!dddir::is_path_exist(m_src_full_path)) {
        ddcout(ddconsole_color::red) << L"src path does not exist: " << m_src_full_path << L"\n";
        return false;
    }

    gen_str();
    if (!insert_into_filters_file(vcxproj_filters_file)) {
        ddcout(ddconsole_color::red) << L"insert_into_filters_file failure\n";
        return false;
    }
    if (!insert_into_vcxproj_file(vcxproj_file)) {
        ddcout(ddconsole_color::red) << L"insert_into_vcxproj_file failure\n";
        return false;
    }
    return true;
}

void project_file_adder::gen_str()
{
    if (!dddir::is_dir(m_src_full_path)) {
        handle_file(m_src_full_path);
        return;
    }

    handle_dir(m_src_full_path);
    dddir::enum_dir(m_src_full_path, [&](const std::wstring& full_path, bool is_dir) {
        if (is_dir) {
            handle_dir(full_path);
        } else {
            handle_file(full_path);
        }
        return false;
    });
}

void project_file_adder::handle_file(const std::wstring& full_path)
{
    std::wstring suffix = ddpath::suffix(full_path);
    if (!is_should_handle_file(suffix)) {
        return;
    }

    if (is_head_file(suffix)) {
        std::string vcxproj_filters_h_str = gen_vcxproj_filters_h_str(full_path);
        m_vcproj_filters_h_vec.push_back(vcxproj_filters_h_str);
        std::string vcxproj_h_str = gen_vcxproj_h_str(full_path);
        m_vcproj_h_vec.push_back(vcxproj_h_str);
    } else {
        std::string vcxproj_filters_cpp_str = gen_vcxproj_filters_cpp_str(full_path);
        m_vcproj_filters_cpp_vec.push_back(vcxproj_filters_cpp_str);
        std::string vcxproj_cpp_str = gen_vcxproj_cpp_str(full_path);
        m_vcproj_cpp_vec.push_back(vcxproj_cpp_str);
    }
}

void project_file_adder::handle_dir(const std::wstring& full_path)
{
    std::string filter_str = gen_vcxproj_filters_filter_str(full_path);
    if (!filter_str.empty()) {
        m_vcproj_filters_filter_vec.push_back(filter_str);
    }
}

std::wstring project_file_adder::get_include_str(const std::wstring& full_path)
{
    if (dddir::is_dir(full_path)) {
        return std::wstring();
    }

    return ddpath::relative_path(m_vcxproj_parent_path, full_path);
}

std::wstring project_file_adder::get_filter_str(const std::wstring& full_path)
{
    if (dddir::is_dir(full_path)) {
        return ddpath::join(m_filter_prefix, ddpath::relative_path(m_src_parent_path, full_path));
    }

    return ddpath::join(m_filter_prefix, ddpath::parent(ddpath::relative_path(m_src_parent_path, full_path)));
}

std::string project_file_adder::gen_vcxproj_filters_cpp_str(const std::wstring& full_path)
{
    std::wstring include_str = get_include_str(full_path);
    std::wstring filter_str = get_filter_str(full_path);
    std::wstring wreturn_str;
    if (filter_str.empty()) {
        wreturn_str = ddstr::format(LR"__(    <ClCompile Include="%s"/>
)__", include_str.c_str());
    } else {
        wreturn_str = ddstr::format(LR"__(    <ClCompile Include="%s">
      <Filter>%s</Filter>
    </ClCompile>
)__", include_str.c_str(), filter_str.c_str());
    }

    return ddstr::utf16_8(wreturn_str);
}

std::string project_file_adder::gen_vcxproj_filters_h_str(const std::wstring& full_path)
{
    std::wstring include_str = get_include_str(full_path);
    std::wstring filter_str = get_filter_str(full_path);
    std::wstring wreturn_str;
    if (filter_str.empty()) {
        wreturn_str = ddstr::format(LR"__(    <ClInclude Include="%s"/>
)__", include_str.c_str());
    } else {
        wreturn_str = ddstr::format(LR"__(    <ClInclude Include="%s">
      <Filter>%s</Filter>
    </ClInclude>
)__", include_str.c_str(), filter_str.c_str());
    }

    return ddstr::utf16_8(wreturn_str);
}

std::string project_file_adder::gen_vcxproj_filters_filter_str(const std::wstring& full_path)
{
    std::wstring filter_str = get_filter_str(full_path);
    if (filter_str.empty()) {
        return std::string();
    }

    std::wstring guid;
    (void)ddguid::generate_guid(guid);
    std::wstring  wreturn_str = ddstr::format(LR"__(    <Filter Include="%s">
      <UniqueIdentifier>%s</UniqueIdentifier>
    </Filter>
)__", filter_str.c_str(), guid.c_str());
    return ddstr::utf16_8(wreturn_str);
}

bool project_file_adder::insert_into_filters_file(const std::shared_ptr<ddfile>& file)
{
    std::list<std::string> lines;
    while (true) {
        std::string line;
        if (!file->read_linea(line)) {
            break;
        }

        lines.push_back(line);
    }

    if (lines.empty()) {
        return false;
    }

    std::list<std::string>::const_iterator last_filter = lines.end();
    std::list<std::string>::const_iterator last_project = lines.end();
    std::list<std::string>::const_iterator last_cicompile = lines.end();
    std::list<std::string>::const_iterator last_cinclude = lines.end();
    for (std::list<std::string>::const_iterator it = lines.begin();
        it != lines.end(); ++it) {
        if (it->find("</Project>") != std::string::npos) {
            last_project = it;
        } else if (it->find("</Filter>") != std::string::npos) {
            last_filter = it;
            auto tmp = it;
            ++tmp;
            if (tmp != lines.end() && tmp->find("</ItemGroup>") != std::string::npos) {
                last_filter = tmp;
            }
        } else if (it->find("</ClCompile>") != std::string::npos ||
            (it->find("<ClCompile") != std::string::npos && it->find("/>") != std::string::npos)) {
            auto tmp = it;
            ++tmp;
            if (tmp != lines.end() && tmp->find("</ItemGroup>") != std::string::npos) {
                last_cicompile = tmp;
            }
        } else if (it->find("</ClInclude>") != std::string::npos ||
            (it->find("<ClInclude") != std::string::npos && it->find("/>") != std::string::npos)) {
            auto tmp = it;
            ++tmp;
            if (tmp != lines.end() && tmp->find("</ItemGroup>") != std::string::npos) {
                last_cinclude = tmp;
            }
        }
    }

    if (last_project == lines.end() || last_project == lines.begin()) {
        return false;
    }

    if (!m_vcproj_filters_filter_vec.empty()) {
        if (last_filter != lines.end()) {
            lines.insert(last_filter, m_vcproj_filters_filter_vec.begin(), m_vcproj_filters_filter_vec.end());
        } else {
            lines.insert(last_project, "  <ItemGroup>\r\n");
            lines.insert(last_project, m_vcproj_filters_filter_vec.begin(), m_vcproj_filters_filter_vec.end());
            lines.insert(last_project, "  </ItemGroup>\r\n");
        }
    }

    if (!m_vcproj_filters_cpp_vec.empty()) {
        if (last_cicompile != lines.end()) {
            lines.insert(last_cicompile, m_vcproj_filters_cpp_vec.begin(), m_vcproj_filters_cpp_vec.end());
        } else {
            lines.insert(last_project, "  <ItemGroup>\r\n");
            lines.insert(last_project, m_vcproj_filters_cpp_vec.begin(), m_vcproj_filters_cpp_vec.end());
            lines.insert(last_project, "  </ItemGroup>\r\n");
        }
    }

    if (!m_vcproj_filters_h_vec.empty()) {
        if (last_cinclude != lines.end()) {
            lines.insert(last_cinclude, m_vcproj_filters_h_vec.begin(), m_vcproj_filters_h_vec.end());
        } else {
            lines.insert(last_project, "  <ItemGroup>\r\n");
            lines.insert(last_project, m_vcproj_filters_h_vec.begin(), m_vcproj_filters_h_vec.end());
            lines.insert(last_project, "  </ItemGroup>\r\n");
        }
    }

    file->seek(0);
    for (auto& it : lines) {
        file->write((const u8*)(it.data()), (s32)it.size());
    }
    return true;
}

std::string project_file_adder::gen_vcxproj_cpp_str(const std::wstring& full_path)
{
    std::wstring include_str = get_include_str(full_path);
    std::wstring  wreturn_str = ddstr::format(LR"__(    <ClCompile Include="%s" />
)__", include_str.c_str());
    return ddstr::utf16_8(wreturn_str);
}

std::string project_file_adder::gen_vcxproj_h_str(const std::wstring& full_path)
{
    std::wstring include_str = get_include_str(full_path);
    std::wstring  wreturn_str = ddstr::format(LR"__(    <ClInclude Include="%s" />
)__", include_str.c_str());
    return ddstr::utf16_8(wreturn_str);
}

bool project_file_adder::insert_into_vcxproj_file(const std::shared_ptr<ddfile>& file)
{
    std::list<std::string> lines;
    while (true) {
        std::string line;
        if (!file->read_linea(line)) {
            break;
        }

        lines.push_back(line);
    }

    if (lines.empty()) {
        return false;
    }

    std::list<std::string>::const_iterator last_project = lines.end();
    std::list<std::string>::const_iterator last_cicompile = lines.end();
    std::list<std::string>::const_iterator last_cinclude = lines.end();
    for (std::list<std::string>::const_iterator it = lines.begin();
        it != lines.end(); ++it) {
        if (it->find("</Project>") != std::string::npos) {
            last_project = it;
        } else if (it->find("</ClCompile>") != std::string::npos ||
            (it->find("<ClCompile") != std::string::npos && it->find("/>") != std::string::npos)) {
            auto tmp = it;
            ++tmp;
            if (tmp != lines.end() && tmp->find("</ItemGroup>") != std::string::npos) {
                last_cicompile = tmp;
            }
        } else if (it->find("</ClInclude>") != std::string::npos ||
            (it->find("<ClInclude") != std::string::npos && it->find("/>") != std::string::npos)) {
            auto tmp = it;
            ++tmp;
            if (tmp != lines.end() && tmp->find("</ItemGroup>") != std::string::npos) {
                last_cinclude = tmp;
            }
        }
    }

    if (last_project == lines.end() || last_project == lines.begin()) {
        return false;
    }

    if (!m_vcproj_cpp_vec.empty()) {
        if (last_cicompile != lines.end()) {
            lines.insert(last_cicompile, m_vcproj_cpp_vec.begin(), m_vcproj_cpp_vec.end());
        } else {
            lines.insert(last_project, "  <ItemGroup>\r\n");
            lines.insert(last_project, m_vcproj_cpp_vec.begin(), m_vcproj_cpp_vec.end());
            lines.insert(last_project, "  </ItemGroup>\r\n");
        }
    }

    if (!m_vcproj_h_vec.empty()) {
        if (last_cinclude != lines.end()) {
            lines.insert(last_cinclude, m_vcproj_h_vec.begin(), m_vcproj_h_vec.end());
        } else {
            lines.insert(last_project, "  <ItemGroup>\r\n");
            lines.insert(last_project, m_vcproj_h_vec.begin(), m_vcproj_h_vec.end());
            lines.insert(last_project, "  </ItemGroup>\r\n");
        }
    }

    // utf-8 head_size
    file->resize(0);
    for (auto& it : lines) {
        file->write((const u8*)(it.data()), (s32)it.size());
    }
    return true;
}

} // namespace NSP_DD
