#include "ddbase/stdafx.h"
#include "ddbase/ddenvironment.h"
#include "ddbase/ddlocale.h"
#include "ddbase/windows/ddmoudle_utils.h"
namespace NSP_DD {
bool ddenvironment::is_big_endian()
{
    u32 num = 0x01020304;
    u8* buffer = reinterpret_cast<u8*>(&num);
    return buffer[0] == 1;
}

bool ddenvironment::set_utf8_locale_and_io_codepage()
{
    return ddlocale::set_utf8_locale_and_io_codepage();
}

bool ddenvironment::set_current_directory(const std::wstring& path)
{
    return ddmoudle_utils::set_current_directory(path);
}
} // namespace NSP_DD
