#ifndef ddbase_ddcolor_h_
#define ddbase_ddcolor_h_

#include "ddbase/dddef.h"
namespace NSP_DD {
#pragma pack(push, 1) 
struct ddbgra
{
    u8 b;
    u8 g;
    u8 r;
    u8 a;
};

struct ddargb
{
    u8 a;
    u8 r;
    u8 g;
    u8 b;
};

struct ddrgb
{
    u8 r;
    u8 g;
    u8 b;
};
#pragma pack(pop)

bool operator==(const ddbgra& l, const ddbgra& r);
bool operator==(const ddargb& l, const ddargb& r);
bool operator==(const ddrgb& l, const ddrgb& r);
} // namespace NSP_DD
#endif // ddbase_ddcolor_h_
