#include "ddbase/stdafx.h"
#include "ddbase/windows/ddcom_utils.h"
#include "ddbase/windows/ddregister.h"

namespace NSP_DD {

std::wstring map_thread_model(com_thread_model model)
{
    switch (model)
    {
    case com_thread_model::Apartment:
        return L"Apartment";
    case com_thread_model::Both:
        return L"Both";
    case com_thread_model::Free:
        return L"Free";
    case com_thread_model::Neutral:
        return L"Neutral";
    }
    return L"";
}

HRESULT write_com_init_register(const std::wstring& clsid, const std::wstring& desc, com_thread_model threadModel, const std::wstring dllFullPath)
{
    LSTATUS status = ERROR_SUCCESS;
    do {
        DDHKEY filterKey;
        std::wstring clsidKeyStr = L"CLSID\\" + clsid;
        status = ::RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidKeyStr.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, NULL, &filterKey, NULL);
        if (ERROR_SUCCESS != status) {
            break;
        }

        status = ::RegSetValueExW(filterKey, NULL, 0, REG_SZ, (const BYTE*)desc.c_str(), (DWORD)desc.size() * 2);
        if (ERROR_SUCCESS != status) {
            break;
        }

        DDHKEY inprocServer32Key;
        status = ::RegCreateKeyExW(filterKey, L"InProcServer32", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &inprocServer32Key, NULL);
        if (ERROR_SUCCESS != status) {
            break;
        }

        status = ::RegSetValueExW(inprocServer32Key, NULL, 0, REG_SZ, (const BYTE*)dllFullPath.c_str(), (DWORD)dllFullPath.size() * 2);
        if (ERROR_SUCCESS != status) {
            break;
        }

        std::wstring threadModelStr = map_thread_model(threadModel);
        status = RegSetValueExW(inprocServer32Key, L"ThreadingModel", 0, REG_SZ, (const BYTE*)threadModelStr.c_str(), (DWORD)threadModelStr.size() * 2);
        if (ERROR_SUCCESS != status) {
            break;
        }
    } while (0);

    if (ERROR_SUCCESS != status) {
        write_com_uninit_register(clsid);
    }

    return HRESULT_FROM_WIN32(status);
}

HRESULT write_com_uninit_register(const std::wstring& clsid)
{
    std::wstring clsidKeyStr = L"CLSID\\" + clsid;
    return HRESULT_FROM_WIN32(::RegDeleteTree(HKEY_CLASSES_ROOT, clsidKeyStr.c_str()));
}

bool NSP_DD::com_has_register(const std::wstring& clsid)
{
    std::wstring clsidKeyStr = L"CLSID\\" + clsid;
    HKEY clsidKey = NULL;
    LSTATUS status = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, clsidKeyStr.c_str(), REG_OPTION_OPEN_LINK, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, &clsidKey);
    if (ERROR_SUCCESS != status || clsidKey == NULL) {
        return false;
    }

    ::RegCloseKey(clsidKey);
    clsidKey = NULL;
    return true;
}

// HRESULT write_dshow_filter_init_register(const dshow_register_desc& desc)
// {
//     LSTATUS status = ERROR_SUCCESS;
//     do {
//         DDHKEY filterKey;
//         std::wstring clsidKeyStr = L"CLSID\\" + desc.categroy + L"\\Instance\\" + desc.clsid;
//         status = ::RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidKeyStr.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, NULL, &filterKey, NULL);
//         if (ERROR_SUCCESS != status) {
//             break;
//         }
// 
//         status = RegSetValueExW(filterKey, L"CLSID", 0, REG_SZ, (const BYTE*)desc.clsid.c_str(), (DWORD)desc.clsid.size() * 2);
//         if (ERROR_SUCCESS != status) {
//             break;
//         }
// 
//         status = RegSetValueExW(filterKey, L"FilterData", 0, REG_SZ, (const BYTE*)desc.filterData.data(), (DWORD)desc.filterData.size());
//         if (ERROR_SUCCESS != status) {
//             break;
//         }
// 
//         status = RegSetValueExW(filterKey, L"FriendlyName", 0, REG_SZ, (const BYTE*)desc.friendlyName.c_str(), (DWORD)desc.friendlyName.size() * 2);
//         if (ERROR_SUCCESS != status) {
//             break;
//         }
//     } while (0);
// 
//     if (ERROR_SUCCESS != status) {
//         write_dshow_filter_uninit_register(desc.categroy, desc.clsid);
//     }
// 
//     return HRESULT_FROM_WIN32(status);
// }
// 
// HRESULT write_dshow_filter_uninit_register(const std::wstring& categroy, const std::wstring& clsid)
// {
//     std::wstring clsidKeyStr = L"CLSID\\" + categroy + L"\\Instance\\" + clsid;
//     return HRESULT_FROM_WIN32(::RegDeleteTree(HKEY_CLASSES_ROOT, clsidKeyStr.c_str()));
// }
// 
// bool read_dshow_filter_register(dshow_register_desc& desc)
// {
//     DDHKEY filterKey;
//     std::wstring clsidKeyStr = L"CLSID\\" + desc.categroy + L"\\Instance\\" + desc.clsid;
//     if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_CLASSES_ROOT, clsidKeyStr.c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &filterKey)) {
//         return false;
//     }
// 
//     {
//         DWORD type = 0;
//         DWORD dataLen = 0;
//         if (::RegQueryValueExW(filterKey, L"FilterData", NULL, &type, NULL, &dataLen) == ERROR_SUCCESS && dataLen != 0) {
//             desc.filterData.resize(dataLen);
//             (void)::RegQueryValueExW(filterKey, L"FilterData", NULL, &type, desc.filterData.data(), &dataLen);
//         }
//     }
// 
//     {
//         DWORD type = 0;
//         DWORD dataLen = 0;
//         if (::RegQueryValueExW(filterKey, L"FriendlyName", NULL, &type, NULL, &dataLen) == ERROR_SUCCESS && dataLen != 0) {
//             dataLen += 2;
//             desc.friendlyName.resize(dataLen / 2);
//             (void)::RegQueryValueExW(filterKey, L"FriendlyName", NULL, &type, (u8*)desc.friendlyName.data(), &dataLen);
//         }
//     }
// 
//     return true;
// }
} // namespace NSP_DD

