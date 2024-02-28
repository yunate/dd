#ifndef ddbase_file_ddpath_h_
#define ddbase_file_ddpath_h_

#include "ddbase/dddef.h"
#include <string>

namespace NSP_DD {
const wchar_t ddpath_separator = L'\\';
class ddpath
{
public:
    static std::wstring name(const std::wstring& path);

    // 去除所有重复的分割符并将其替换为ddpath_separator
    // 去除末尾的分隔符
    static void normal(std::wstring& path, wchar_t separator = ddpath_separator);
    static std::wstring normal1(const std::wstring& path, wchar_t separator = ddpath_separator);

    // 1. normal
    // 2. 展开所有的..和. 直到无法展开为止 如 a/b/../../../c -> ../c
    static void expand(std::wstring& path, wchar_t separator = ddpath_separator);
    static std::wstring expand1(const std::wstring& path, wchar_t separator = ddpath_separator);

    // 合并路径
    static std::wstring join(const std::wstring& l, const std::wstring& r, wchar_t separator = ddpath_separator);
    static std::wstring join(const std::vector<std::wstring>& l, wchar_t separator = ddpath_separator);

    // 获得后缀
    static std::wstring suffix(const std::wstring& path);

    // 获得父目录, 返回值不包含分割符
    static std::wstring parent(const std::wstring& path);

    // 获得src_full_path相对于base_full_path的相对路径
    // e.g.
    // base_full_path = d:\a\b\e
    //  src_full_path = d:\a\b\c\d\e\f
    // 返回值 = ..\c\d\e\f
    // e.g.
    // base_full_path = d:\a
    //  src_full_path = c:\a\b\c\d\e\f
    // 返回值 = ..\..\c:\a\b\c\d\e\f
    static std::wstring relative_path(const std::wstring& base_full_path, const std::wstring& src_full_path, wchar_t separator = ddpath_separator);
};
} // namespace NSP_DD
#endif // ddbase_file_ddpath_h_
