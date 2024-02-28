#ifndef ddbase_pickle_ddidata_parser_h_
#define ddbase_pickle_ddidata_parser_h_
#include "ddbase/dddef.h"

namespace NSP_DD {
enum class dddata_parse_state
{
    // 解析失败
    error = 0,
    // 部分解析成功,需要更多数据
    need_more_data,
    // 调用者处理玩数据,继续调用函数解析
    continue_parse,
    // 解析完成
    complete,
};

struct dddata_parse_result
{
    dddata_parse_state parse_state = dddata_parse_state::error;
    // 本次parse消费的字符数量
    s32 parsed_size = 0;

    inline void reset()
    {
        parse_state = dddata_parse_state::error;
        parsed_size = 0;
    }
};

class ddidata_parser
{
public:
    virtual ~ddidata_parser() = default;
    // 从buff中解析
    // @note buff 和 buff_size 由调用者保证有效
    // @note continue_parse_from_buff应该将解析后的数据放到context中,调用get_context获得
    virtual const dddata_parse_result& continue_parse_from_buff(u8* buff, s32 buff_size) = 0;
    virtual const void* get_context() = 0;
    virtual void reset() = 0;

public:
    // 没有上下文的解析可以直接封装成函数
    // 获得以\n结尾的一行
    static void get_nline_from_buff(u8* buff, s32 buff_size, std::string& line, dddata_parse_result& result);
    // 获得以\r\n结尾的一行
    static void get_rnline_from_buff(u8* buff, s32 buff_size, std::string& line, dddata_parse_result& result);
    // 获得以指定字符结尾的一行
    static void get_xline_from_buff(u8* buff, s32 buff_size, const std::string& finder, std::string& line, dddata_parse_result& result);
};

} // namespace NSP_DD
#endif // ddbase_pickle_ddidata_parser_h_
