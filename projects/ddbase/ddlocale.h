#ifndef ddbase_ddlocale_h_
#define ddbase_ddlocale_h_

#include "ddbase/dddef.h"

namespace NSP_DD {
    // code_page_id: https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
    // local_name: https://learn.microsoft.com/zh-cn/cpp/c-runtime-library/locale-names-languages-and-country-region-strings?view=msvc-170
    class ddlocale
    {
    public:
        // 将io的codepage和locale都设置为utf8, 这样可以解决乱码问题
        static bool set_utf8_locale_and_io_codepage();

        // 获得当前程序locale的code_page id
        // 当前程序启动时候会调用setlocale( LC_ALL, "C" );
        // https://learn.microsoft.com/zh-cn/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170#:~:text=setlocale(%20LC_ALL%2C%20%22C%22%20)%3B
        // 所以当前的codepage和iocodepage/系统codepage不一致
        static u32 get_current_locale_codepage_id();
        static bool set_current_locale(const std::string& name);

        // 获得/设置当前程序io的code_page
        // 当前程序io的code_page指的是控制台显示字符串的编码
        // 当前程序默认io的code_page编码和get_system_codepage的一致
        static s32 get_io_codepage();
        static bool set_io_codepage(s32 code_page);

        // 获得当前系统的codepage
        // 当前系统的codepage指的是操作系统的默认编码, 比如中文为GBK等
        static s32 get_system_codepage();
    };
} // namespace NSP_DD
#endif // ddbase_ddlocale_h_
