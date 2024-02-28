#ifndef ddtools_sln_maker_sln_maker_helper_h_
#define ddtools_sln_maker_sln_maker_helper_h_

#include "ddbase/ddmini_include.h"
#include <string>
namespace NSP_DD {

class ddsln_maker_helper
{
public:
    static bool copy_and_replace(
        const std::wstring& templete_path,
        const std::wstring& dst_path, 
        const std::vector<std::string>& finder,
        const std::vector<std::string>& replacer,
        const std::function<bool(const std::wstring&)>& filter = nullptr);
    static bool expand_macro(
        const std::wstring& file_path,
        const std::vector<std::string>& macros,
        const std::vector<std::string>& macros_def = {"###ifdef", "###ifndef", "###else", "###endif"});
};

} // namespace NSP_DD

#endif // ddtools_sln_maker_sln_maker_helper_h_
