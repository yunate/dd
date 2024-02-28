#ifndef ddbase_windows_ddmoudle_utils_h_
#define ddbase_windows_ddmoudle_utils_h_

#include "ddbase/dddef.h"
#include <windows.h>

namespace NSP_DD {
class ddmoudle_utils
{
public:
    /** 根据路径获得句柄
    @param [in] moudle_name
        大小写不敏感;
        加载的模块的名称(.dll或.exe文件);
        对于模块名称, 如果省略文件扩展名, 则会追加默认库扩展名.dll;
        文件名字符串可以包含尾随点字符(.), 以指示模块名称没有扩展名;
        字符串不必指定路径, 指定路径时,请务必使用反斜杠(\), 而不是正斜杠(/);
        如果此参数为 NULL, 则该函数将返回用于创建调用进程(.exe 文件)的文件的句柄.
    @return 失败返回NULL, 否则返回相应的句柄, 不要尝试释放该句柄
    */
    static HMODULE get_moudleA(const std::string& moudle_name = "");
    static HMODULE get_moudleW(const std::wstring& moudle_name = L"");

    /** 根据句柄返回全路径
    @param [in] moudle 如果此参数为 NULL, 则该函数将返回用于创建调用进程(.exe 文件)的文件路径.
    @return 失败返回空字符串, 否则返回相应的路径
    */
    static std::string get_moudle_pathA(HMODULE moudle = NULL);
    static std::wstring get_moudle_pathW(HMODULE moudle = NULL);

    // 设置当前进程的工作目录, 如果path为空, 则设置为当前模块的路径
    static bool set_current_directory(const std::wstring& path);
};
} // namespace NSP_DD
#endif // ddbase_windows_ddmoudle_utils_h_

