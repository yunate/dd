#include "ddbase/stdafx.h"
#include "ddbase/windows/ddsecurity.h"
#include <sddl.h>

namespace NSP_DD {

bool ddsecurity::create_low_sa(SECURITY_ATTRIBUTES& secAttr)
{
    secAttr.nLength = sizeof(SECURITY_DESCRIPTOR);

    // "S:(ML;;NW;;;LW)" this means "low integrity"
    PSECURITY_DESCRIPTOR pSD = NULL;
    if (!::ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;LW)", SDDL_REVISION_1, &pSD, NULL)) {
        return false;
    }

    BOOL fSaclPresent = FALSE;
    PACL pSacl = NULL;
    BOOL fSaclDefaulted = FALSE;
    if (!::GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted)) {
        return false;
    }

    if (!::InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION)) {
        return false;
    }

    if (!::SetSecurityDescriptorSacl(secAttr.lpSecurityDescriptor, TRUE, pSacl, FALSE)) {
        return false;
    }

    return true;
}
} // namespace NSP_DD