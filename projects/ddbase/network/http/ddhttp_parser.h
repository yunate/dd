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
    ddhttp_parser(const ddparser_read_pred& read_pred);

    // 在head_parsed完成后会返回need_read, 在每次读取body数据后会返回need_read
    // 完成后返回complete
    ddcoroutine<dddata_parse_state> continue_parse(ddexpire expire) override;

    struct ddhttp_parser_context
    {
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
    ddhttp_parser_context* get_context_t();
    const void* get_context() override;
    void reset() override;

#ifdef _DEBUG
    bool pre_parse_complete()
    {
        return m_parse_step == ddhttp_parse_step::parse_begin;
    }
#endif

private:
    ddcoroutine<bool> read(ddexpire expire);
    dddata_parse_state parse_header_first_line();
    dddata_parse_state parse_header_kvs();
    dddata_parse_state parse_body_size();
    dddata_parse_state parse_body();
    dddata_parse_state parse_chunked_end_line();
    dddata_parse_state parse_trailer();

private:
    enum class ddhttp_parse_step
    {
        parse_begin = 0,
        parse_head_first_line,
        parse_head_kvs,
        parse_body_size,
        parse_body,
        parse_chunked_end_line, // \r\n
        parse_trailer,
        parse_end,
    };

    // buff
    bool m_need_read = false;
    std::vector<u8> m_parser_buff;
    u8* m_remain_buff_point = nullptr;
    s32 m_buff_remain_size = 0;

    // parser context
    ddhttp_parse_step m_parse_step = ddhttp_parse_step::parse_begin;
    ddhttp_parser_context m_context;
    bool m_is_chunked = false;
    s32 m_chunked_size = 0;
    s32 m_body_recv_size = 0;
};

using ddhttp_response_parser = ddhttp_parser<ddhttp_response_header>;
using ddhttp_request_parser = ddhttp_parser<ddhttp_request_header>;
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_parser_h_