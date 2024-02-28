
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/ddprocess.h"
#include "ddbase/ddrandom.h"

namespace NSP_DD {
DDTEST(test_win_utils, ddstr_guid)
{
    std::string guidStr = "{431ECE1A-B4EB-4C15-8899-CAB52D13A06A}";
    GUID guid;
    bool b1 = ddguid::ddstr_guid(guidStr, guid); b1;
    std::string guidStrO;
    bool bb1 = ddguid::ddguid_str(guid, guidStrO); bb1;

    std::string guidStr2 = "{431ECE1A-B4EB-1125-8899-CAB52D13A06}";
    GUID guid2;
    bool b2 = ddguid::ddstr_guid(guidStr2, guid2); b2;
    std::string guidStrO2;
    bool bb2 = ddguid::ddguid_str(guid2, guidStrO2); bb2;

    std::string guidStr3 = "";
    GUID guid3;
    bool b3 = ddguid::ddstr_guid(guidStr3, guid3); b3;
    std::string guidStrO3;
    bool bb3 = ddguid::ddguid_str(guid3, guidStrO3); bb3;

    std::string guidStr4 = "{0000000A-000B-0005-0009-CAB52D13000A}";
    GUID guid4;
    bool b4 = ddguid::ddstr_guid(guidStr4, guid4); b4;
    std::string guidStrO4;
    bool bb4 = ddguid::ddguid_str(guid4, guidStrO4); bb4;
}

DDTEST(test_win_utils, ddstr_guidw)
{
    std::wstring guidStr = L"{431ECE1A-B4EB-4C15-8899-CAB52D13A06A}";
    GUID guid;
    bool b1 = ddguid::ddstr_guid(guidStr, guid); b1;
    std::wstring guidStrO;
    bool bb1 = ddguid::ddguid_str(guid, guidStrO); bb1;

    std::wstring guidStr2 = L"{431ECE1A-B4EB-4C15-8899-CAB52D13A06}";
    GUID guid2;
    bool b2 = ddguid::ddstr_guid(guidStr2, guid2); b2;
    std::wstring guidStrO2;
    bool bb2 = ddguid::ddguid_str(guid2, guidStrO2); bb2;

    std::wstring guidStr3 = L"";
    GUID guid3;
    bool b3 = ddguid::ddstr_guid(guidStr3, guid3); b3;
    std::wstring guidStrO3;
    bool bb3 = ddguid::ddguid_str(guid3, guidStrO3); bb3;
}

} // namespace NSP_DD
