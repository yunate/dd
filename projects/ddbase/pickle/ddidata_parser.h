#ifndef ddbase_pickle_ddidata_parser_h_
#define ddbase_pickle_ddidata_parser_h_
#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/ddtime.h"
#include <functional>

namespace NSP_DD {
enum class dddata_parse_state
{
    // 解析失败
    error = 0,
    // 继续解析
    need_read,
    // 解析完成
    complete,
};

class ddidata_parser
{
public:
    using ddparser_read_pred = std::function<ddcoroutine<std::tuple<bool, s32>>(void* buff, s32 buff_size, ddexpire expire)>;
    ddidata_parser(const ddparser_read_pred& read_pred);
    virtual ~ddidata_parser() = default;
    // 此函数应该在循环中调用, 直到返回complete/error
    // @return 返回error/complete 表示解析失败/完成, 此时需要终止循环; 返回need_read表示有数据需要读取后继续解析, 调用get_context获取解析的数据
    // @note continue_parse_from_buff应该将解析后的数据放到context中,调用get_context获得
    virtual ddcoroutine<dddata_parse_state> continue_parse(ddexpire expire) = 0;
    virtual const void* get_context() = 0;
    virtual void reset() = 0;

public:
    // 返回直到\n的字符数量, 如果返回-1, 则表示未找到需要更多数据, e.g. buff = "123\n456", 返回3
    static s32 get_nline_from_buff(u8* buff, s32 buff_size);
    // 返回直到\r\n的字符数量, 如果返回-1, 则表示未找到需要更多数据, e.g. buff = "123\r\n456", 返回3
    static s32 get_rnline_from_buff(u8* buff, s32 buff_size);
    // 返回直到finder的字符数量, 如果返回-1, 则表示未找到需要更多数据, e.g. buff = "123--456"; finder = "--", 返回3
    static s32 get_xline_from_buff(u8* buff, s32 buff_size, const std::string& finder);

protected:
    ddparser_read_pred m_read_pred;
};

} // namespace NSP_DD
#endif // ddbase_pickle_ddidata_parser_h_
