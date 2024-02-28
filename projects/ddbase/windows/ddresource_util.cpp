#include "ddbase/stdafx.h"
#include "ddbase/windows/ddresource_util.h"
#include "ddbase/file/ddfile.h"
#include <memory>
namespace NSP_DD {

bool ddresource_util::release_res(HMODULE hModule, DWORD resId, const std::wstring& resTy, const std::wstring& target)
{
    HRSRC hrc = ::FindResource(hModule, MAKEINTRESOURCE(resId), resTy.c_str());
    if (hrc == nullptr) {
        return false;
    }

    HGLOBAL hGlobal = ::LoadResource(hModule, hrc);
    if (hGlobal == nullptr) {
        return false;
    }

    DWORD dwSize = ::SizeofResource(hModule, hrc);
    LPVOID pData = ::LockResource(hGlobal);
    std::shared_ptr<ddfile> file(ddfile::create_utf8_file(target));
    if (file == nullptr) {
        return false;
    }

    return file->write((u8*)pData, dwSize);
}
} // namespace NSP_DD
