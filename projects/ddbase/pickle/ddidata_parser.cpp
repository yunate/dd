#include "ddbase/stdafx.h"
#include "ddbase/pickle/ddidata_parser.h"
namespace NSP_DD {
ddidata_parser::ddidata_parser(const ddparser_read_pred& read_pred)
    : m_read_pred(read_pred)
{
    DDASSERT(m_read_pred != nullptr);
}

s32 ddidata_parser::get_nline_from_buff(u8* buff, s32 buff_size)
{
    for (s32 i = 0; i < buff_size; ++i) {
        if (buff[i] == '\n') {
            return i;
        }
    }

    return -1;
}

s32 ddidata_parser::get_rnline_from_buff(u8* buff, s32 buff_size)
{
    for (s32 i = 0; i < buff_size - 1; ++i) {
        if (buff[i] == '\r' && buff[i + 1] == '\n') {
            return i;
        }
    }

    return -1;
}

s32 ddidata_parser::get_xline_from_buff(u8* buff, s32 buff_size, const std::string& finder)
{
    for (s32 i = 0; i < buff_size + 1 - (s32)finder.size(); ++i) {
        bool match = true;
        for (s32 j = 0; j < (s32)finder.size(); ++j) {
            if (buff[j + i] != finder[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            return i;
        }
    }

    return -1;
}
} // namespace NSP_DD
