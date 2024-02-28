#ifndef ddtools_code_format_ddcode_snapshot_cpp_h_
#define ddtools_code_format_ddcode_snapshot_cpp_h_

#include "ddbase/ddmini_include.h"

#include <stack>

namespace NSP_DD {
struct ddcode_snapshot_cpp_line_desc
{
    s32 line_number = -1;
    std::string raw_str;

    // pretty_str = raw_str - 注释 - 多余空格(包括首尾和中间多余的空格) - '\r' - '\n'
    std::string pretty_str;
};

class ddcode_snapshot_cpp_file
{
public:
    ~ddcode_snapshot_cpp_file();
    bool analyze_file(const std::wstring& full_path);
    const std::vector<ddcode_snapshot_cpp_line_desc*>& get_desc();
private:
    void handle_space(std::string& line, ddcode_snapshot_cpp_line_desc* desc);
    void handle_comment(std::string& line, ddcode_snapshot_cpp_line_desc* desc);

    std::vector<ddcode_snapshot_cpp_line_desc*> m_descs;
    std::stack<s32> m_comments;
};
}
#endif // ddtools_code_format_ddcode_snapshot_cpp_h_
