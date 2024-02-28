#include "ddbase/stdafx.h"
#include "ddbase/windows/ddmoudle_utils.h"
#include "ddbase/file/ddpath.h"

namespace NSP_DD {
HMODULE ddmoudle_utils::get_moudleA(const std::string& moudle_name /* "" */)
{
    if (moudle_name.empty()) {
        return ::GetModuleHandleA(NULL);
    }
    return ::GetModuleHandleA(moudle_name.c_str());
}
HMODULE  ddmoudle_utils::get_moudleW(const std::wstring& moudle_name /* L"" */)
{
    if (moudle_name.empty()) {
        return ::GetModuleHandleW(NULL);
    }
    return ::GetModuleHandleW(moudle_name.c_str());
}

std::string ddmoudle_utils::get_moudle_pathA(HMODULE moudle /* = NULL */)
{
    if (moudle == NULL) {
        moudle = get_moudleA("");
        if (moudle == NULL) {
            return "";
        }
    }

    const u32 buffsize = 1024;
    char buff[buffsize];
    u32 size = (u32)::GetModuleFileNameA(moudle, buff, buffsize);
    if (size == 0) {
        return "";
    }
    if (size == buffsize || size == 0) {
        return "";
    }

    return buff;
}
std::wstring ddmoudle_utils::get_moudle_pathW(HMODULE moudle /* = NULL */)
{
    if (moudle == NULL) {
        moudle = get_moudleW(L"");
        if (moudle == NULL) {
            return L"";
        }
    }

    const u32 buffsize = 1024;
    wchar_t buff[buffsize];
    u32 size = (u32)::GetModuleFileNameW(moudle, buff, buffsize);
    if (size == 0) {
        return L"";
    }
    if (size == buffsize || size == 0) {
        return L"";
    }

    return buff;
}

bool ddmoudle_utils::set_current_directory(const std::wstring& path)
{
    std::wstring dir = path;
    if (dir.empty()) {
        dir = get_moudle_pathW();
    }
    if (dir.empty()) {
        return false;
    }

    dir = ddpath::parent(dir);
    if (!::SetCurrentDirectoryW(dir.c_str())) {
        return false;
    }
    return true;
}
} // namespace NSP_DD
