#ifndef ddbase_network_http_ddhttp_utils_h_
#define ddbase_network_http_ddhttp_utils_h_
#include "ddbase/dddef.h"

namespace NSP_DD {
class ddhttp_utils
{
public:
    // 根据文件后缀获得Content-Type
    // html json xml jpeg mp4 pdf css js ...
    static std::string get_content_type(const std::string& suffix);
};
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_utils_h_
