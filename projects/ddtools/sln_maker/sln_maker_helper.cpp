#include "ddtools/stdafx.h"
#include "ddtools/sln_maker/sln_maker_helper.h"
#include "ddbase/file/ddfile.h"
#include "ddbase/file/dddir.h"
#include "ddbase/file/ddpath.h"
#include "ddbase/str/ddstr.h"
#include <memory>
#include <stack>
namespace NSP_DD {

static void strings_buffs(const std::vector<std::string>& src, std::vector<ddbuff>& buff)
{
    buff.resize(src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        buff[i].resize(src[i].size());
        (void)::memcpy_s(buff[i].data(), src[i].size(), src[i].data(), src[i].size());
    }
}

static bool copy_and_replace_file(const std::wstring& templete_file_path, const std::wstring& dst_file_path, const std::vector<ddbuff>& finder, const std::vector<ddbuff>& replacer)
{
    std::shared_ptr<ddfile> templete_file(ddfile::create_utf8_file(templete_file_path));
    if (templete_file == nullptr) {
        return false;
    }

    std::shared_ptr<ddfile> dst_file(ddfile::create_utf8_file(dst_file_path));
    if (dst_file == nullptr) {
        return false;
    }

    return ddfile::file_replace(templete_file.get(), dst_file.get(), finder, replacer);
}

bool ddsln_maker_helper::copy_and_replace(
    const std::wstring& templete_path,
    const std::wstring& dst_path,
    const std::vector<std::string>& finder,
    const std::vector<std::string>& replacer,
    const std::function<bool(const std::wstring&)>& filter /* = nullptr */)
{
    if (finder.size() != replacer.size()) {
        return false;
    }

    if (!dddir::is_path_exist(templete_path)) {
        return false;
    }

    std::vector<ddbuff> src_buff;
    std::vector<ddbuff> dst_buff;
    strings_buffs(finder, src_buff);
    strings_buffs(replacer, dst_buff);
    if (!dddir::is_dir(templete_path)) {
        return copy_and_replace_file(templete_path, dst_path, src_buff, dst_buff);
    }

    std::vector<std::wstring> finderw(finder.size());
    std::vector<std::wstring> replacerw(replacer.size());
    for (size_t i = 0; i < finder.size(); ++i) {
        ddstr::utf8_16(finder[i], finderw[i]);
        ddstr::utf8_16(replacer[i], replacerw[i]);
    }

    bool successful = false;
    dddir::enum_dir(templete_path, [&](const std::wstring& sub_path, const std::wstring& name, bool isDir) {
        std::wstring raw_path = ddpath::join({ templete_path, sub_path, name });
        if (filter != nullptr && !filter(raw_path)) {
            // skip this file
            return false;
        }
        
        std::wstring new_path = ddpath::join({ dst_path, sub_path, name });
        for (size_t i = 0; i < finderw.size(); ++i) {
            new_path = ddstr::replace(new_path.c_str(), finderw[i].c_str(), replacerw[i].c_str());
        }
        if (isDir) {
            successful = dddir::create_dir_ex(new_path);
        } else {
            successful = copy_and_replace_file(raw_path, new_path, src_buff, dst_buff);
        }
        return !successful;
    });

    return successful;
}

static bool macro_cantains(const std::vector<std::string>& macros, const std::string& str)
{
    std::string tmp = str;
    ddstr::trim(tmp, std::vector<char>{' ', '\r', '\n'});
    for (const auto& it : macros) {
        if (tmp == it) {
            return true;
        }
    }
    return false;
}

bool ddsln_maker_helper::expand_macro(
    const std::wstring& file_path,
    const std::vector<std::string>& macros,
    const std::vector<std::string>& macros_def /* = {"###ifdef", "###ifndef", "###else", "###endif"} */)
{
    DDASSERT_FMTW(macros_def.size() == 4, LR"__(the macros_def's size should be 4, e.g. ["#ifdef", "#ifndef", "#else", "#endif"])__");
    const std::string& macro_ifdef = macros_def[0];
    const std::string& macro_ifndef = macros_def[1];
    const std::string& macro_else = macros_def[2];
    const std::string& macro_endif = macros_def[3];

    std::shared_ptr<ddfile> file(ddfile::create_utf8_file(file_path));
    if (file == nullptr) {
        return false;
    }

    std::vector<std::string> lines;
    bool skip = false;
    int pre_should_skip_depth = -1;
    std::stack<int> should_skip_depth_stack;
    int depth = 0;
    std::string line;
    while (file->read_linea(line)) {
        std::string tmp = line;
        ddstr::trim(tmp);

        if (ddstr::beginwith(tmp.c_str(), macro_ifdef.c_str())) {
            ++depth;
            if (skip) {
                continue;
            }
            tmp = tmp.substr(macro_ifdef.size());
            ddstr::trim(tmp);
            if (!macro_cantains(macros, tmp)) {
                should_skip_depth_stack.push(pre_should_skip_depth);
                pre_should_skip_depth = depth;
                skip = true;
            }
            continue;
        }

        if (ddstr::beginwith(tmp.c_str(), macro_ifndef.c_str())) {
            ++depth;
            if (skip) {
                continue;
            }
            tmp = tmp.substr(macro_ifndef.size());
            ddstr::trim(tmp);
            if (macro_cantains(macros, tmp)) {
                should_skip_depth_stack.push(pre_should_skip_depth);
                pre_should_skip_depth = depth;
                skip = true;
            }
            continue;
        }

        if (ddstr::beginwith(tmp.c_str(), macro_else.c_str())) {
            if (skip) {
                if (depth == pre_should_skip_depth) {
                    skip = false;
                }
            } else {
                skip = true;
                should_skip_depth_stack.push(pre_should_skip_depth);
                pre_should_skip_depth = depth;
            }

            continue;
        }

        if (ddstr::beginwith(tmp.c_str(), macro_endif.c_str())) {
            if (depth == 0) {
                // 多一个#endif, 比如第一行就是#endif
                continue;
            }

            if (depth == pre_should_skip_depth) {
                pre_should_skip_depth = should_skip_depth_stack.top();
                should_skip_depth_stack.pop();
                skip = false;
            }
            --depth;
            continue;
        }

        if (!skip) {
            lines.push_back(line);
        }
    }

    file->seek(0);
    for (const auto& it : lines) {
        if (!file->write((u8*)it.c_str(), (s32)it.size())) {
            return false;
        }
    }
    return file->resize(file->current_offset());
}

} // namespace NSP_DD

