#ifndef ddbase_ddenvironment_h_
#define ddbase_ddenvironment_h_

#include "ddbase/dddef.h"

namespace NSP_DD {
class ddenvironment
{
public:
    // 一般的windows机器是小端
    static bool is_big_endian();

    // 参考ddlocale::set_utf8_locale_and_io_codepage
    static bool set_utf8_locale_and_io_codepage();

    // 参考ddmoudle_utils::set_current_directory
    static bool set_current_directory(const std::wstring& path);
};
} // namespace NSP_DD
#endif // ddbase_ddenvironment_h_
