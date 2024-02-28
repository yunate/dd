#include "ddbase/stdafx.h"
#include "ddbase/pickle/ddidata_parser.h"
namespace NSP_DD {
void ddidata_parser::get_nline_from_buff(u8* buff, s32 buff_size, std::string& line, dddata_parse_result& result)
{
    line.clear();
    for (s32 i = 0; i < buff_size; ++i) {
        if (buff[i] == '\n') {
            result.parse_state = dddata_parse_state::complete;
            result.parsed_size = i + 1;

            if (i > 0) {
                line.assign((char*)buff, i);
            }
            return;
        }
    }

    result.parse_state = dddata_parse_state::need_more_data;
}

void ddidata_parser::get_rnline_from_buff(u8* buff, s32 buff_size, std::string& line, dddata_parse_result& result)
{
    line.clear();
    for (s32 i = 0; i < buff_size - 1; ++i) {
        if (buff[i] == '\r' && buff[i + 1] == '\n') {
            result.parse_state = dddata_parse_state::complete;
            result.parsed_size = i + 2;

            if (i > 0) {
                line.assign((char*)buff, i);
            }
            return;
        }
    }

    result.parse_state = dddata_parse_state::need_more_data;
}

void ddidata_parser::get_xline_from_buff(u8* buff, s32 buff_size, const std::string& finder, std::string& line, dddata_parse_result& result)
{
    line.clear();
    for (s32 i = 0; i < buff_size + 1 - (s32)finder.size(); ++i) {
        bool match = true;
        for (s32 j = 0; j < (s32)finder.size(); ++j) {
            if (buff[j + i] != finder[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            if (i > 0) {
                line.assign((char*)buff, i);
            }
            return;
        }
    }

    result.parse_state = dddata_parse_state::need_more_data;
}
} // namespace NSP_DD
