#include "ddtools/stdafx.h"
#include "ddtools/code_format/ddcode_snapshot_cpp.h"
#include "ddbase/ddio.h"
#include "ddbase/ddlog.hpp"

namespace NSP_DD {
ddcode_snapshot_cpp_file::~ddcode_snapshot_cpp_file()
{
    for (auto& it : m_descs) {
        DDASSERT(it != nullptr);
        delete it;
    }
    m_descs.clear();
}

bool ddcode_snapshot_cpp_file::analyze_file(const std::wstring& full_path)
{
    std::shared_ptr<ddfile> file(ddfile::create_utf8_file(full_path));
    if (file == nullptr) {
        DDLOG_LASTERROR()
        return false;
    }

    s32 line_number = 0;
    while (true) {
        std::string line;
        if (!file->read_linea(line)) {
            break;
        }

        ddcode_snapshot_cpp_line_desc* desc = new ddcode_snapshot_cpp_line_desc();
        desc->raw_str = line;
        desc->line_number = ++line_number;

        handle_space(line, desc);
        handle_comment(line, desc);

        m_descs.push_back(desc);
    }

    return true;
}

const std::vector<ddcode_snapshot_cpp_line_desc*>& ddcode_snapshot_cpp_file::get_desc()
{
    return m_descs;
}

void ddcode_snapshot_cpp_file::handle_space(std::string& line, ddcode_snapshot_cpp_line_desc*)
{
    ddstr::trim(line, std::vector<char>{ ' ', '\t', '\r', '\n' });
    if (line.empty()) {
        return;
    }

    size_t pos = 0;
    for (size_t i = 1; i < line.size(); ++i) {
        if (line[i] == ' ' || line[i] == '\t') {
            if (line[pos] == ' ' || line[pos] == '\t') {
                continue;
            }
        }
        line[++pos] = line[i];
    }

    line.resize(pos + 1);
}

void ddcode_snapshot_cpp_file::handle_comment(std::string& line, ddcode_snapshot_cpp_line_desc* desc)
{
    if (line.empty()) {
        return;
    }

    for (size_t i = 0; i < line.size() - 1; ++i) {
        char c = line[i];
        char c1 = line[i + 1];

        if (!m_comments.empty()) {
            line[i] = 0;
        }

        if (c == '/' && c1 == '/') {
            line.resize(i);
            break;
        }

        if (c == '/' && c1 == '*') {
            m_comments.push(desc->line_number);
            line[i] = 0;
            line[i + 1] = 0;
            ++i;
            continue;
        }

        if (c == '*' && c1 == '/') {
            if (!m_comments.empty()) {
                m_comments.pop();
                ++i;
            } else {
                // error, miss `/*` before `*/`
            }
            continue;
        }
    }

    std::string tmp;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] != 0) {
            tmp.append(1, line[i]);
        }
    }
    line = tmp;
    desc->pretty_str = line;
}
}