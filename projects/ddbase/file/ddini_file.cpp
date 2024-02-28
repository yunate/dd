#include "ddbase/stdafx.h"
#include "ddbase/file/ddini_file.h"
#include "ddbase/file/dddir.h"
#include <Windows.h>

namespace NSP_DD {
ddini_file::ddini_file(const std::wstring& sPath)
    : m_sFileFullPath(sPath) {}

ddini_file* ddini_file::create_obj(const std::wstring& sPath, bool bAlwaysCreate)
{
    if (!dddir::is_path_exist(sPath)) {
        if (!bAlwaysCreate) {
            return nullptr;
        }

        if (!dddir::create_file(sPath)) {
            return nullptr;
        }
    }

    return new(std::nothrow) ddini_file(sPath);
}

bool ddini_file::add_key(const std::wstring& section_name, const std::wstring& key, const std::wstring& value)
{
    if (section_name.empty() || key.empty() || value.empty()) {
        return false;
    }
    return 0 == ::WritePrivateProfileString(section_name.c_str(), key.c_str(), value.c_str(), m_sFileFullPath.c_str());
}

bool ddini_file::delete_key(const std::wstring& section_name, const std::wstring& key)
{
    if (section_name.empty() || key.empty()) {
        return false;
    }
    return 0 == ::WritePrivateProfileString(section_name.c_str(), key.c_str(), NULL, m_sFileFullPath.c_str());
}

bool ddini_file::delete_section(const std::wstring& section_name)
{
    if (section_name.empty()) {
        return false;
    }
    return 0 == ::WritePrivateProfileString(section_name.c_str(), NULL, NULL, m_sFileFullPath.c_str());
}

static bool get_buff(std::vector<WCHAR>& buff, std::function<s32(std::vector<WCHAR>&)> func)
{
    s32 buff_size = 1024;
    while (true) {
        buff.resize(buff_size);
        s32 size = func(buff);
        if (size == 0) {
            return false;
        }

        if (size == -1) {
            // buffer is not enough
            buff_size *= 2;
        } else {
            break;
        }

        if (buff_size == 1024 * 1024) {
            return false;
        }
    }
    return true;
}

bool ddini_file::get_all_section_name(std::vector<std::wstring>& section_names)
{
    std::vector<WCHAR> buff;
    if (!get_buff(buff, [this](std::vector<WCHAR>& buff) {
        DWORD size = ::GetPrivateProfileString(NULL, NULL, NULL, buff.data(), (DWORD)buff.size(), m_sFileFullPath.c_str());
        if (size == (DWORD)buff.size() - 2) {
            return -1;
        }
        return (s32)size;
    })) {
        return false;
    }

    WCHAR* tmp = buff.data();
    while (*tmp != 0) {
        std::wstring section(tmp);
        section_names.push_back(section);
        tmp += section.length() + 1;
    }
    return !section_names.empty();
}

bool ddini_file::get_all_key_name(const std::wstring& section_name, std::vector<std::wstring>& key_names)
{
    if (section_name.empty()) {
        return false;
    }
    std::vector<WCHAR> buff;
    if (!get_buff(buff, [this, &section_name](std::vector<WCHAR>& buff) {
        DWORD size = ::GetPrivateProfileString(section_name.c_str(), NULL, NULL, buff.data(), (DWORD)buff.size(), m_sFileFullPath.c_str());
        if (size == (DWORD)buff.size() - 2) {
            return -1;
        }
        return (s32)size;
    })) {
        return false;
    }

    WCHAR* tmp = buff.data();
    while (*tmp != 0) {
        std::wstring section(tmp);
        key_names.push_back(section);
        tmp += section.length() + 1;
    }
    return !key_names.empty();
}

bool ddini_file::get_value(const std::wstring& section_name, const std::wstring& key_name, std::wstring& value)
{
    if (section_name.empty() || key_name.empty()) {
        return false;
    }
    std::vector<WCHAR> buff;
    if (!get_buff(buff, [this, &section_name, &key_name](std::vector<WCHAR>& buff) {
        DWORD size = ::GetPrivateProfileString(section_name.c_str(), key_name.c_str(), NULL, buff.data(), (DWORD)buff.size(), m_sFileFullPath.c_str());
        if (size == (DWORD)buff.size() - 1) {
            return -1;
        }
        return (s32)size;
    })) {
        return false;
    }

    value = buff.data();
    return !value.empty();
}

} // namespace NSP_DD
