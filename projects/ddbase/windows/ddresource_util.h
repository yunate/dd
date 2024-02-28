#ifndef ddbase_windows_ddresource_util_h_
#define ddbase_windows_ddresource_util_h_

#include "ddbase/dddef.h"
#include <windows.h>
namespace NSP_DD {

class ddresource_util
{
    // 释放资源文件
    // hModule ： 模块句柄，比如wWinMain( _In_ HINSTANCE hInstance,...) 的hInstance
    // target ： 保存路径
    bool release_res(HMODULE hModule, DWORD resId, const std::wstring& resTy, const std::wstring& target);
};

} // namespace NSP_DD
#endif // ddbase_windows_ddresource_util_h_

