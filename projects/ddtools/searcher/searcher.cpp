#include "ddtools/stdafx.h"
#include "ddtools/searcher/searcher.h"
#include "ddbase/ddcmd_line_utils.h"
#include "ddbase/ddio.h"
#include "ddbase/str/ddstr.h"
#include "ddbase/file/dddir.h"
#include "ddbase/ddsys.h"

namespace NSP_DD {
searcher::~searcher() {
    for (const auto& it : m_result) {
        delete it;
    }

    m_result.clear();
    if (m_pool != nullptr) {
        m_pool->stop_all();
        delete m_pool;
    }
}

void searcher::run()
{
    u32 thread_count = get_thread_count();
    if (thread_count > 1) {
        thread_count -= 1;
    }
    m_pool = new ddtask_thread_pool(thread_count, 10);
    std::vector<std::wstring> cmds;
    ddcmd_line_utils::get_cmds(cmds);
    if (!process_cmds(cmds)) {
        return;
    }

    search_dir(m_src_path);
    write_result();
}

void searcher::write_result()
{
    std::unique_ptr<ddfile> file;
    if (!m_out_file.empty()) {
        if (dddir::is_path_exist(m_out_file) && !dddir::is_dir(m_out_file)) {
            dddir::delete_path(m_out_file);
        }
        file.reset(ddfile::create_utf8_file(m_out_file));
    }

    for (auto* it : m_result) {
        for (const auto& info : it->line_line_infos) {
            std::string line = ddstr::format("%s:%d|%d\n", ddstr::utf16_8(it->file_path).c_str(), info.line_number, info.pos);
            if (file != nullptr) {
                file->write((u8*)line.c_str(), (s32)line.length());
            } else {
                ddcout(ddconsole_color::green) << line;
            }
        }
    }
}

void searcher::help()
{
    ddcout(ddconsole_color::gray) << L"searcher.exe src_path search_word [out_file_path]\r\n";
}

bool searcher::process_cmds(const std::vector<std::wstring>& cmds)
{
    if (cmds.size() < 3) {
        help();
        return false;
    }

    m_src_path = cmds[1];
    m_target = ddstr::utf16_8(cmds[2]);

    if (cmds.size() > 3) {
        m_out_file = cmds[3];
    }

    return true;
}

void searcher::search_file(const std::wstring& file_path)
{
    if (ddstr::endwith(file_path.c_str(), L".exe") ||
        ddstr::endwith(file_path.c_str(), L".pdb")) {
        return;
    }

    while (true) {
        bool push_result = m_pool->push_task([file_path, this]() {
            if (!dddir::is_path_exist(file_path)) {
                return;
            }

            std::unique_ptr<ddfile> file(ddfile::create_utf8_file(file_path));
            if (file != nullptr) {
                auto* result = search_in_utf8_file(std::move(file));
                if (result != nullptr) {
                    m_mutex.lock();
                    m_result.push_back(result);
                    m_mutex.unlock();
                }
                return;
            }


            file.reset(ddfile::create_usc2_file(file_path));
            if (file != nullptr) {
                auto* result = search_in_utf16_file(std::move(file));
                if (result != nullptr) {
                    m_mutex.lock();
                    m_result.push_back(result);
                    m_mutex.unlock();
                }
                return;
            }
        });
        if (!push_result) {
            (void)m_pool->wait_ready_to_push();
        } else {
            break;
        }
    }
}

searcher::search_result* searcher::search_in_utf8_file(std::unique_ptr<ddfile> file)
{
    std::string line;
    search_result* result = nullptr;
    s32 line_number = 0;
    while (file->read_linea(line)) {
        ++line_number;
        size_t offset = 0;
        while (true) {
            offset = line.find(m_target.c_str(), offset);
            if (offset == std::string::npos) {
                break;
            }
            if (result == nullptr) {
                result = new search_result();
                result->file_path = file->get_fullpath();
            }
            result->line_line_infos.push_back({ line_number, (s32)offset });
            offset += (s32)m_target.length();
        }
    }

    return result;
}

searcher::search_result* searcher::search_in_utf16_file(std::unique_ptr<ddfile> file)
{
    std::wstring target = ddstr::utf8_16(m_target);
    std::wstring line;
    search_result* result = nullptr;
    s32 line_number = 0;
    while (file->read_linew(line)) {
        ++line_number;
        size_t offset = 0;
        while (true) {
            offset = line.find(target.c_str(), offset);
            if (offset == std::string::npos) {
                break;
            }
            if (result == nullptr) {
                result = new search_result();
                result->file_path = file->get_fullpath();
            }
            result->line_line_infos.push_back({ line_number, (s32)offset });
        }
    }
    return result;
}

void searcher::search_dir(const std::wstring& dir_path)
{
    std::queue<std::wstring> dirs;
    dirs.push(dir_path);
    while (!dirs.empty()) {
        std::wstring path = dirs.front();
        dirs.pop();
        std::vector<dddir::enum_dir_info> result;
        dddir::enum_dir_first_level(path, result);
        for (const auto& it : result) {
            if (it.name == L"" || it.name == L"out" || it.name[0] == '.' || it.name == L"third_party") {
                continue;
            }
            std::wstring full_path = ddpath::join(path, it.name);
            if (it.is_dir) {
                dirs.push(full_path);
            } else {
                search_file(full_path);
            }
        }
    }
}
}