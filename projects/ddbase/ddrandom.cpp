
#include "ddbase/stdafx.h"
#define _CRT_RAND_S
#include "ddbase/ddrandom.h"
#include "ddbase/str/ddstr.h"
#include <stdlib.h>
#include <ObjBase.h>
namespace NSP_DD {
bool ddrandom::get_rand_number(int min, int max, int& out)
{
    if (max < min) {
        return false;
    }

    unsigned int tmp = 0;

    if (::rand_s(&tmp) != 0) {
        return false;
    }

    out = tmp % (max - min + 1) + min;
    return true;
}

//////////////////////////////////////GUID////////////////////////////////////
bool ddguid::ddstr_guid(const std::wstring& guidStr, ::GUID& guid)
{
    short d0, d1, d2, d3, d4, d5, d6, d7;
    int cnt = ::swscanf_s(guidStr.c_str(), L"{%08X-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}",
        &(guid.Data1), &(guid.Data2), &(guid.Data3), &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7);
    guid.Data4[0] = (unsigned char)d0;
    guid.Data4[1] = (unsigned char)d1;
    guid.Data4[2] = (unsigned char)d2;
    guid.Data4[3] = (unsigned char)d3;
    guid.Data4[4] = (unsigned char)d4;
    guid.Data4[5] = (unsigned char)d5;
    guid.Data4[6] = (unsigned char)d6;
    guid.Data4[7] = (unsigned char)d7;
    return cnt == 11;
}

bool ddguid::ddstr_guid(const std::string& guidStr, ::GUID& guid)
{
    short d0, d1, d2, d3, d4, d5, d6, d7;
    int cnt = ::sscanf_s(guidStr.c_str(), "{%08X-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}",
        &(guid.Data1), &(guid.Data2), &(guid.Data3), &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7);
    guid.Data4[0] = (unsigned char)d0;
    guid.Data4[1] = (unsigned char)d1;
    guid.Data4[2] = (unsigned char)d2;
    guid.Data4[3] = (unsigned char)d3;
    guid.Data4[4] = (unsigned char)d4;
    guid.Data4[5] = (unsigned char)d5;
    guid.Data4[6] = (unsigned char)d6;
    guid.Data4[7] = (unsigned char)d7;
    return cnt == 11;
}

bool ddguid::ddguid_str(const ::GUID& guid, std::wstring& guidStr)
{
    guidStr.resize(64);
    int cnt = ::swprintf_s(&(guidStr[0]), guidStr.length(), L"{%08X-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    guidStr = guidStr.c_str();
    return cnt == 38;
}

bool ddguid::ddguid_str(const ::GUID& guid, std::string& guidStr)
{
    guidStr.resize(64);
    int cnt = ::sprintf_s(&(guidStr[0]), guidStr.length(), "{%08X-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    guidStr = guidStr.c_str();
    return cnt == 38;
}


bool ddguid::generate_guid(std::string& guid)
{
    ::GUID g;
    if (S_OK != ::CoCreateGuid(&g)) {
        return false;
    }

    return ddguid_str(g, guid);
}

bool ddguid::generate_guid(std::wstring& guid)
{
    ::GUID g;
    if (S_OK != ::CoCreateGuid(&g)) {
        return false;
    }

    return ddguid_str(g, guid);
}


} // namespace NSP_DD
