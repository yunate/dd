#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddenvironment.h"

namespace NSP_DD {
DDTEST(test_case_ddenvironment_is_big_endian, 1)
{
    bool is_big_endian = ddenvironment::is_big_endian();
    u32 num = 0x01020304;
    u8* buffer = reinterpret_cast<u8*>(&num);
    if (is_big_endian) {
        DDASSERT(buffer[0] == 1);
    } else {
        DDASSERT(buffer[0] != 1);
    }
}
} // namespace NSP_DD
