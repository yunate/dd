#include "ddbase/stdafx.h"
#include "ddbase/network/http/ddhttp_parser.h"
#include "ddbase/str/ddstr.h"
namespace NSP_DD {

template <class T>
ddhttp_parser<T>::ddhttp_parser(const ddparser_read_pred& read_pred)
    : ddidata_parser(read_pred)
{ }

template <class T>
void ddhttp_parser<T>::reset()
{
    m_parse_step = ddhttp_parse_step::parse_begin;
    m_context.reset();
    m_is_chunked = false;
    m_chunked_size = 0;
    m_body_recv_size = 0;
    m_need_read = false;
    m_parser_buff.resize(0);
}

template<class T>
const void* ddhttp_parser<T>::get_context()
{
    return &m_context;
}

template<class T>
typename ddhttp_parser<T>::ddhttp_parser_context* ddhttp_parser<T>::get_context_t()
{
    return (ddhttp_parser<T>::ddhttp_parser_context*)get_context();
}

template <class T>
ddcoroutine<dddata_parse_state> ddhttp_parser<T>::continue_parse(ddexpire expire)
{
    m_context.body_buff = nullptr;
    m_context.body_buff_size = 0;

    while (true) {
        if (m_need_read) {
            if (!co_await read(expire)) {
                co_return dddata_parse_state::error;
            }
        }

        dddata_parse_state current_parsed_result = dddata_parse_state::complete;
        switch (m_parse_step) {
        case ddhttp_parse_step::parse_begin: {
            reset();
            m_parse_step = ddhttp_parse_step::parse_head_first_line;
            break;
        }
        case ddhttp_parse_step::parse_head_first_line: {
            current_parsed_result = parse_header_first_line();
            break;
        }
        case ddhttp_parse_step::parse_head_kvs: {
            current_parsed_result = parse_header_kvs();
            break;
        }
        case ddhttp_parse_step::parse_body_size: {
            current_parsed_result = parse_body_size();
            break;
        }
        case ddhttp_parse_step::parse_body: {
            current_parsed_result = parse_body();
            break;
        }
        case ddhttp_parse_step::parse_chunked_end_line: {
            current_parsed_result = parse_chunked_end_line();
            break;
        }
        case ddhttp_parse_step::parse_trailer: {
            current_parsed_result = parse_trailer();
            break;
        }
        case ddhttp_parse_step::parse_end: {
            // here successful means all data have been parsed successfully.
            m_parse_step = ddhttp_parse_step::parse_begin;
            co_return dddata_parse_state::complete;
        }
        }

        if (current_parsed_result != dddata_parse_state::complete) {
            // 如果 current_parsed_result != dddata_parse_state::complete, 意味着有数据需要caller取出或者出现错误
            co_return current_parsed_result;
        }
    }
}

template<class T>
ddcoroutine<bool> ddhttp_parser<T>::read(ddexpire expire)
{
    DDASSERT(m_read_pred != nullptr);
    m_need_read = false;

    if (m_parser_buff.empty()) {
        m_parser_buff.resize(2048);
    }

    if (m_buff_remain_size > 0) {
        if (m_buff_remain_size + 512 >= (s32)m_parser_buff.size()) {
            if (m_parser_buff.size() >= 2 * 1024 * 1024) {
                // when the buff is genter than 2MB, we do not ty to read but threat it as failure.
                DDASSERT(false);
                co_return false;
            }
            m_parser_buff.resize((s32)m_parser_buff.size() + 512);
        }
        (void)::memmove(m_parser_buff.data(), m_remain_buff_point, m_buff_remain_size);
    }

    auto read_result = co_await m_read_pred(m_parser_buff.data() + m_buff_remain_size, (s32)m_parser_buff.size() - m_buff_remain_size, expire);
    if (!std::get<0>(read_result)) {
         co_return false;
    }

    m_remain_buff_point = (u8*)m_parser_buff.data();
    m_buff_remain_size += std::get<1>(read_result);
    co_return true;
}

template<>
dddata_parse_state ddhttp_parser<ddhttp_request_header>::parse_header_first_line()
{
    s32 size = ddidata_parser::get_rnline_from_buff(m_remain_buff_point, m_buff_remain_size);
    if (size == -1) {
        m_need_read = true;
        return dddata_parse_state::complete;
    }

    std::string line((const char*)m_remain_buff_point, size);
    m_buff_remain_size -= (size + 2);
    m_remain_buff_point += (size + 2);

    std::vector<std::string> out;
    ddstr::split(line.c_str(), " ", out);
    if (out.size() != 3) {
        return dddata_parse_state::error;
    }

    m_context.header.method = out[0];
    m_context.header.uri = out[1];
    m_context.header.version = out[2];
    m_parse_step = ddhttp_parse_step::parse_head_kvs;
    return dddata_parse_state::complete;
}

template<>
dddata_parse_state ddhttp_parser<ddhttp_response_header>::parse_header_first_line()
{
    s32 size = ddidata_parser::get_rnline_from_buff(m_remain_buff_point, m_buff_remain_size);
    if (size == -1) {
        m_need_read = true;
        return dddata_parse_state::complete;
    }

    std::string line((const char*)m_remain_buff_point, size);
    m_buff_remain_size -= (size + 2);
    m_remain_buff_point += (size + 2);

    // version
    size_t pos = line.find(' ');
    if (pos == std::string::npos) {
        return dddata_parse_state::error;
    }

    m_context.header.version = line.substr(0, pos);
    line = line.substr(pos + 1);

    // state
    pos = line.find(' ');
    if (pos == std::string::npos) {
        return dddata_parse_state::error;
    }

    m_context.header.state = std::atoi(line.substr(0, pos).c_str());
    m_context.header.state_str = line.substr(pos + 1);
    m_parse_step = ddhttp_parse_step::parse_head_kvs;
    return dddata_parse_state::complete;
}

template<class T>
dddata_parse_state ddhttp_parser<T>::parse_header_kvs()
{
    while (true) {
        s32 size = ddidata_parser::get_rnline_from_buff(m_remain_buff_point, m_buff_remain_size);
        if (size == -1) {
            m_need_read = true;
            return dddata_parse_state::complete;
        }

        std::string line((const char*)m_remain_buff_point, size);
        m_buff_remain_size -= (size + 2);
        m_remain_buff_point += (size + 2);

        if (line.empty()) {
            // empty line, this indicate the end of header
            m_context.head_parsed = true;
            m_is_chunked = m_context.header.kvs.is_chunked();
            m_parse_step = ddhttp_parse_step::parse_body_size;
            return dddata_parse_state::need_read;
        }

        // key-value
        size_t pos = line.find(':');
        if (pos == std::string::npos) {
            // error line, skip it and do not treat as an error.
            return dddata_parse_state::error;
        }

        std::string key = line.substr(0, pos);
        ddstr::trim(key);
        std::string value;
        if (pos != line.size() - 1) {
            value = line.substr(pos + 1);
            ddstr::trim(value);
        }

        if (!key.empty() && !value.empty()) {
            m_context.header.kvs.append(key, value);
        }
    }
}

template <class T>
dddata_parse_state ddhttp_parser<T>::parse_body_size()
{
    m_body_recv_size = 0;
    m_chunked_size = 0;

    if (!m_is_chunked) {
        m_chunked_size = m_context.header.kvs.content_lenth();
        m_parse_step = ddhttp_parse_step::parse_body;
        return dddata_parse_state::complete;
    } else {
        // get chunked size
        s32 size = ddidata_parser::get_rnline_from_buff(m_remain_buff_point, m_buff_remain_size);
        if (size == -1) {
            if (m_buff_remain_size >= 6) {
                // FF FF FF FF\r\n, 如果大于6个字符, 则认为是错误的chunked size
                return dddata_parse_state::error;
            } else {
                m_need_read = true;
                return dddata_parse_state::complete;
            }
        }

        std::string line((const char*)m_remain_buff_point, size);
        m_buff_remain_size -= (size + 2);
        m_remain_buff_point += (size + 2);

        // 16 进制
        m_chunked_size = ::strtol(line.c_str(), nullptr, 16);
        m_body_recv_size = 0;
        m_parse_step = ddhttp_parse_step::parse_body;
        return dddata_parse_state::complete;
    }
}

template <class T>
dddata_parse_state ddhttp_parser<T>::parse_body()
{
    if (m_chunked_size == 0) {
        if (!m_is_chunked) {
            m_parse_step = ddhttp_parse_step::parse_trailer;
        } else {
            m_parse_step = ddhttp_parse_step::parse_chunked_end_line;
        }

        return dddata_parse_state::complete;
    }

    s32 need_size = m_chunked_size - m_body_recv_size;
    s32 parsed_size = min(need_size, m_buff_remain_size);
    m_context.body_buff = m_remain_buff_point;
    m_context.body_buff_size = parsed_size;
    m_body_recv_size += parsed_size;

    m_buff_remain_size -= parsed_size;
    m_remain_buff_point += parsed_size;

    if (m_chunked_size > m_body_recv_size) {
        m_need_read = true;
        return dddata_parse_state::need_read;
    }

    if (m_is_chunked) {
        // do not return complete but need_read to let the caller get the body data.
        m_parse_step = ddhttp_parse_step::parse_chunked_end_line;
        return dddata_parse_state::need_read;
    } else {
        m_parse_step = ddhttp_parse_step::parse_trailer;
        return dddata_parse_state::complete;
    }
}

template <class T>
dddata_parse_state ddhttp_parser<T>::parse_chunked_end_line()
{
    DDASSERT(m_is_chunked);

    s32 size = ddidata_parser::get_rnline_from_buff(m_remain_buff_point, m_buff_remain_size);
    if (size == -1) {
        if (m_buff_remain_size >= 2) {
            // \r\n, 如果大于2个字符, 则认为是错误的chunked size
            return dddata_parse_state::error;
        } else {
            m_need_read = true;
            return dddata_parse_state::complete;
        }
    }

    m_buff_remain_size -= (size + 2);
    m_remain_buff_point += (size + 2);

    if (m_chunked_size == 0) {
        // the last chunk
        m_parse_step = ddhttp_parse_step::parse_trailer;
    } else {
        // has more chunk
        m_parse_step = ddhttp_parse_step::parse_body_size;
    }
    return dddata_parse_state::complete;
}

template <class T>
dddata_parse_state ddhttp_parser<T>::parse_trailer()
{
    m_parse_step = ddhttp_parse_step::parse_end;
    return dddata_parse_state::complete;
}

template class ddhttp_parser<ddhttp_request_header>;
template class ddhttp_parser<ddhttp_response_header>;
} // namespace NSP_DD