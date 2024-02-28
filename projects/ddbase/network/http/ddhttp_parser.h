#ifndef ddbase_network_http_ddhttp_parser_h_
#define ddbase_network_http_ddhttp_parser_h_
#include "ddbase/dddef.h"
#include "ddbase/network/http/ddhttp_header.h"
#include "ddbase/pickle/ddidata_parser.h"
#include <memory>

namespace NSP_DD {

template <class HEADER>
class ddhttp_parser : public ddidata_parser
{
public:
    struct ddhttp_parser_context
    {
        dddata_parse_result parse_result;

        // 是否请求/回复头已经parsed, 只有head_parsed为true, 数据才有效
        bool head_parsed = false;
        HEADER header;

        // 解析的body数据, caller可以复制并保存这些数据
        u8* body_buff = nullptr;
        s32 body_buff_size = 0;

        void reset()
        {
            head_parsed = false;
            header.reset();
            body_buff = nullptr;
            body_buff_size = 0;
        }
    };

public:
    // 从buff中解析
    // @param [out] parsed_size 已经解析的字符数
    // @note ddhttp_parser_context::body_buff 将指向buff, 所以调用者应保证使用ddhttp_parser_context::body_buff时候buff有效
    const dddata_parse_result& continue_parse_from_buff(u8* buff, s32 buff_size) override;
    void reset() override;
    const void* get_context() override;
    ddhttp_parser_context* get_context_t();

private:
    void parse_header_first_line_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result);
    void parse_header_kvs_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result);
    void parse_body_size_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result);
    void parse_body_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result);
    void parse_chunked_end_line_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result);
    void parse_trailer_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result);

private:
    enum class ddhttp_parse_step
    {
        parse_head_first_line = 0,
        parse_head_kvs,
        parse_body_size,
        parse_body,
        parse_chunked_end_line, // \r\n
        parse_trailer,
        parse_end
    };

    dddata_parse_result parse_result;
    ddhttp_parse_step m_parse_step = ddhttp_parse_step::parse_head_first_line;

    ddhttp_parser_context m_context;
    bool m_is_chunked = false;
    s32 m_chunked_size = 0;
    s32 m_body_recv_size = 0;
};

using ddhttp_response_parser = ddhttp_parser<ddhttp_response_header>;
using ddhttp_request_parser = ddhttp_parser<ddhttp_request_header>;
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_parser_h_