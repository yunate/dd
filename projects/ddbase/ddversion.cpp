#include "ddbase/stdafx.h"
#include "ddbase/ddversion.h"
#include <Windows.h>

namespace NSP_DD {
ddversion::ddversion(u32 v0, u32 v1, u32 v2, u32 v3)
{
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
    v[3] = v3;
}

u32& ddversion::operator[](int i)
{
    return v[i];
}

const u32& ddversion::operator[](int i) const
{
    return v[i];
}

inline bool ddversion::empty()
{
    return (v[0] == 0) && (v[1] == 0) && (v[2] == 0) && (v[3] == 0);
}


std::string ddversion::str()
{
    return ddstr::format("%d.%d.%d.%d", v[0], v[1], v[2], v[3]);
}

std::wstring ddversion::wstr()
{
    return ddstr::format(L"%d.%d.%d.%d", v[0], v[1], v[2], v[3]);
}

bool operator<(const ddversion& l, const ddversion& r)
{
    if (l[0] != r[0]) {
        return l[0] < r[0];
    }
    if (l[1] != r[1]) {
        return l[1] < r[1];
    }
    if (l[2] != r[2]) {
        return l[2] < r[2];
    }
    if (l[3] != r[3]) {
        return l[3] < r[3];
    }
    return false;
}

bool operator>(const ddversion& l, const ddversion& r)
{
    if (l[0] != r[0]) {
        return l[0] > r[0];
    }
    if (l[1] != r[1]) {
        return l[1] > r[1];
    }
    if (l[2] != r[2]) {
        return l[2] > r[2];
    }
    if (l[3] != r[3]) {
        return l[3] > r[3];
    }
    return false;
}

bool operator<=(const ddversion& l, const ddversion& r)
{
    return !(l > r);
}

bool operator>=(const ddversion& l, const ddversion& r)
{
    return !(l < r);
}

ddversion ddversion::GetWindowsVersion()
{
    static ddversion windowsVersion = DDEMPTY_VERSION;
    if (!windowsVersion.empty()) {
        return windowsVersion;
    }

    ddversion tmpVersion(DDEMPTY_VERSION);
    VS_FIXEDFILEINFO* pvs = nullptr;
    u32 infoSize = ::GetFileVersionInfoSize(L"kernel32", NULL);
    ddbuff data(infoSize);
    if (!GetFileVersionInfo(L"kernel32", 0, infoSize, (LPVOID)data.data())) {
        return DDEMPTY_VERSION;
    }

    if (!VerQueryValue(data.data(), L"\\", (LPVOID*)&pvs, &infoSize)) {
        return DDEMPTY_VERSION;
    }

    tmpVersion = ddversion((u32)HIWORD(pvs->dwFileVersionMS), (u32)HIWORD(pvs->dwFileVersionMS), (u32)HIWORD(pvs->dwFileVersionLS), (u32)LOWORD(pvs->dwFileVersionLS));
    if (tmpVersion[0] == 10) {
        HKEY key;
        if (::RegOpenKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &key) != ERROR_SUCCESS) {
            return DDEMPTY_VERSION;
        }

        DWORD version = 0;
        DWORD size = sizeof(DWORD);
        if (::RegQueryValueExA(key, "UBR", NULL, NULL, (LPBYTE)&version, &size) == ERROR_SUCCESS) {
            if ((u32)version > tmpVersion[3]) {
                tmpVersion[3] = (u32)version;
            }
        }
        ::RegCloseKey(key);
    }

    windowsVersion = tmpVersion;
    return windowsVersion;
}


} // namespace NSP_DD
