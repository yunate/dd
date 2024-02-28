#ifndef ddtools_searcher_searcher_h_
#define ddtools_searcher_searcher_h_

#include "ddbase/ddmini_include.h"
#include "ddbase/file/ddfile.h"
#include "ddbase/thread/ddtask_thread.h"

#include <memory>

namespace NSP_DD {
class searcher
{
public:
    void run();
    ~searcher();

private:
    struct search_result {
        std::wstring file_path;
        struct line_info {
            s32 line_number;
            s32 pos;
        };
        std::vector<line_info> line_line_infos;
    };
    void search_file(const std::wstring& file_path);
    search_result* search_in_utf8_file(std::unique_ptr<ddfile> file);
    search_result* search_in_utf16_file(std::unique_ptr<ddfile> file);
    void search_dir(const std::wstring& dir_path);
    void help();
    bool process_cmds(const std::vector<std::wstring>& cmds);
    void write_result();
    std::string m_target;
    std::wstring m_out_file;
    std::wstring m_src_path;
    std::vector<search_result*> m_result;
    std::mutex m_mutex;
    ddtask_thread_pool* m_pool = nullptr;
};
}
#endif // ddtools_searcher_searcher_h_
