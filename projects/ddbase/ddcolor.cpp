#include "ddbase/stdafx.h"
#include "ddbase/ddcolor.h"
namespace NSP_DD {
bool operator==(const ddbgra& l, const ddbgra& r)
{
    return l.a == r.a && l.r == r.r && l.g == r.g && l.b == r.b;
}

bool operator==(const ddargb& l, const ddargb& r)
{
    return l.a == r.a && l.r == r.r && l.g == r.g && l.b == r.b;
}

bool operator==(const ddrgb& l, const ddrgb& r)
{
    return l.r == r.r && l.g == r.g && l.b == r.b;
}
} // namespace NSP_DD
