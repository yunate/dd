#ifndef ddtools_project_file_adder_project_file_adder_h_
#define ddtools_project_file_adder_project_file_adder_h_

#include "ddbase/ddmini_include.h"

#include <stack>

namespace NSP_DD {


class project_file_adder
{
public:
    project_file_adder(const std::wstring& vcxproj_full_path, const std::wstring& src_full_path, const std::wstring& filter_prefix = L"");
    ~project_file_adder() = default;

    bool add();

private:
    void gen_str();
    void handle_file(const std::wstring& full_path);
    void handle_dir(const std::wstring& full_path);
    std::wstring get_include_str(const std::wstring& full_path);
    std::wstring get_filter_str(const std::wstring& full_path);

    // vcxproj.filters
    std::string gen_vcxproj_filters_cpp_str(const std::wstring& full_path);
    std::string gen_vcxproj_filters_h_str(const std::wstring& full_path);
    std::string gen_vcxproj_filters_filter_str(const std::wstring& full_path);
    bool insert_into_filters_file(const std::shared_ptr<ddfile>& file);

    // vcxproj
    std::string gen_vcxproj_cpp_str(const std::wstring& full_path);
    std::string gen_vcxproj_h_str(const std::wstring& full_path);
    bool insert_into_vcxproj_file(const std::shared_ptr<ddfile>& file);

    std::wstring m_vcxproj_full_path;
    std::wstring m_vcxproj_parent_path;
    std::wstring m_src_full_path;
    std::wstring m_src_parent_path;
    std::wstring m_filter_prefix;

    // vcxproj.filters
    std::vector<std::string> m_vcproj_filters_h_vec;
    std::vector<std::string> m_vcproj_filters_cpp_vec;
    std::vector<std::string> m_vcproj_filters_filter_vec;

    // vcxproj
    std::vector<std::string> m_vcproj_h_vec;
    std::vector<std::string> m_vcproj_cpp_vec;

};
}
#endif // ddtools_project_file_adder_project_file_adder_h_
