#ifndef ddbase_windows_ddsecurity_h_
#define ddbase_windows_ddsecurity_h_

#include "ddbase/dddef.h"
#include <windows.h>
#pragma comment(lib, "Advapi32.lib")

namespace NSP_DD {
class ddsecurity
{
public:
    static bool create_low_sa(SECURITY_ATTRIBUTES& secAttr);
};

} // namespace NSP_DD
#endif // ddbase_windows_ddsecurity_h_
