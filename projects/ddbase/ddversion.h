#ifndef ddbase_ddversion_h_
#define ddbase_ddversion_h_

#include "ddbase/dddef.h"
#include "ddbase/str/ddstr.h"
#pragma comment(lib, "Version.lib")

namespace NSP_DD {

// v0123
class ddversion
{
public:
    static ddversion GetWindowsVersion();

public:
    ddversion(u32 v0, u32 v1, u32 v2, u32 v3);
    u32& operator[](int i);
    const u32& operator[](int i) const;
    inline bool empty();
    std::string str();
    std::wstring wstr();
private:
    u32 v[4];
};

#define DDEMPTY_VERSION ddversion(0, 0, 0, 0)

bool operator<(const ddversion& l, const ddversion& r);
bool operator>(const ddversion& l, const ddversion& r);
bool operator<=(const ddversion& l, const ddversion& r);
bool operator>=(const ddversion& l, const ddversion& r);

#define DDVERSION_MIN_WINXP ddversion(5, 1, 0, 0)
#define DDVERSION_MIN_WIN7 ddversion(6, 1, 0, 0)
#define DDVERSION_MIN_WIN8 ddversion(6, 3, 0, 0)
#define DDVERSION_MIN_WIN10 ddversion(10, 0, 0, 0)
} // namespace NSP_DD
#endif // ddbase_ddversion_h_

