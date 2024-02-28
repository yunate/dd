#include "ddbase/stdafx.h"
#include "ddbase/network/http/ddhttp_parser.h"
#include "ddbase/str/ddstr.h"
namespace NSP_DD {
template <class T>
const dddata_parse_result& ddhttp_parser<T>::continue_parse_from_buff(u8* buff, s32 buff_size)
{
    DDASSERT(buff != nullptr);
    m_context.body_buff = nullptr;
    m_context.body_buff_size = 0;
    dddata_parse_result& result = m_context.parse_result;
    result.reset();
    if (buff_size == 0) {
        return result;
    }

    while (true) {
        dddata_parse_result current_parsed_result;
        switch (m_parse_step) {
        case ddhttp_parse_step::parse_head_first_line: {
            parse_header_first_line_from_buff(buff, buff_size, current_parsed_result);
            break;
        }
        case ddhttp_parse_step::parse_head_kvs: {
            parse_header_kvs_from_buff(buff, buff_size, current_parsed_result);
            break;
        }
        case ddhttp_parse_step::parse_body_size: {
            parse_body_size_from_buff(buff, buff_size, current_parsed_result);
            break;
        }
        case ddhttp_parse_step::parse_body: {
            parse_body_from_buff(buff, buff_size, current_parsed_result);
            break;
        }
        case ddhttp_parse_step::parse_chunked_end_line: {
            parse_chunked_end_line_from_buff(buff, buff_size, current_parsed_result);
            break;
        }
        case ddhttp_parse_step::parse_trailer: {
            parse_trailer_from_buff(buff, buff_size, current_parsed_result);
            break;
        }
        case ddhttp_parse_step::parse_end: {
            // here successful means all data have been parsed successfully.
            return result;
        }
        }

        result.parsed_size += current_parsed_result.parsed_size;
        result.parse_state = current_parsed_result.parse_state;

        // here complete means the buff have been complete parsed pre step
        // and the buff have more data to continue parse.
        if (result.parse_state != dddata_parse_state::complete) {
            return result;
        }

        buff += current_parsed_result.parsed_size;
        buff_size -= current_parsed_result.parsed_size;
    }
}

template <class T>
void ddhttp_parser<T>::reset()
{
    m_context.reset();
    m_parse_step = ddhttp_parse_step::parse_head_first_line;
    m_is_chunked = false;
    m_chunked_size = 0;
    m_body_recv_size = 0;
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

template<>
void ddhttp_parser<ddhttp_request_header>::parse_header_first_line_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result)
{
    std::string line;
    ddidata_parser::get_rnline_from_buff(buff, buff_size, line, result);
    if (result.parse_state != dddata_parse_state::complete) {
        return;
    }

    std::vector<std::string> out;
    ddstr::split(line.c_str(), " ", out);
    if (out.size() != 3) {
        result.parse_state = dddata_parse_state::error;
        return;
    }

    m_context.header.method = out[0];
    m_context.header.uri = out[1];
    m_context.header.version = out[2];
    m_parse_step = ddhttp_parse_step::parse_head_kvs;
}

template<>
void ddhttp_parser<ddhttp_response_header>::parse_header_first_line_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result)
{
    std::string line;
    ddidata_parser::get_rnline_from_buff(buff, buff_size, line, result);
    if (result.parse_state != dddata_parse_state::complete) {
        return;
    }

    // version
    size_t pos = line.find(' ');
    if (pos == std::string::npos) {
        result.parse_state = dddata_parse_state::error;
        return;
    }

    m_context.header.version = line.substr(0, pos);
    line = line.substr(pos + 1);

    // state
    pos = line.find(' ');
    if (pos == std::string::npos) {
        result.parse_state = dddata_parse_state::error;
        return;
    }

    m_context.header.state = std::atoi(line.substr(0, pos).c_str());
    m_context.header.state_str = line.substr(pos + 1);
    m_parse_step = ddhttp_parse_step::parse_head_kvs;
}

template<class T>
void ddhttp_parser<T>::parse_header_kvs_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result)
{
    std::string line;
    dddata_parse_result current_result;
    while (true) {
        current_result.reset();
        ddidata_parser::get_rnline_from_buff(buff + result.parsed_size, buff_size - result.parsed_size, line, current_result);
        result.parsed_size += current_result.parsed_size;
        result.parse_state = current_result.parse_state;
        if (result.parse_state != dddata_parse_state::complete) {
            return;
        }

        if (line.empty()) {
            // empty line, this indicate the end of header
            m_parse_step = ddhttp_parse_step::parse_body_size;
            m_context.head_parsed = true;
            m_is_chunked = m_context.header.kvs.is_chunked();
            return;
        }

        // key-value
        size_t pos = line.find(':');
        if (pos == std::string::npos) {
            // error line, skip it and do not treat as an error.
            return;
        }

        std::string key = line.substr(0, pos);
        std::string value;
        if (pos != line.size() - 1) {
            value = line.substr(pos + 1);
            ddstr::trim(value);
        }

        m_context.header.kvs.append(key, value);
    }
}

template <class T>
void ddhttp_parser<T>::parse_body_size_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result)
{
    m_body_recv_size = 0;
    m_chunked_size = 0;

    if (!m_is_chunked) {
        m_chunked_size = m_context.header.kvs.content_lenth();
        result.parse_state = dddata_parse_state::complete;
    } else {
        // get chunked size
        std::string line;
        ddidata_parser::get_rnline_from_buff(buff, buff_size, line, result);
        if (result.parse_state != dddata_parse_state::complete) {
            // FF FF FF FF\r\n, 如果大于6个字符, 则认为是错误的chunked size
            if (buff_size >= 6) {
                result.parse_state = dddata_parse_state::error;
            }
            return;
        }

        // 16 进制
        m_chunked_size = ::strtol(line.c_str(), nullptr, 16);
        m_body_recv_size = 0;
    }

    m_parse_step = ddhttp_parse_step::parse_body;
}

template <class T>
void ddhttp_parser<T>::parse_body_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result)
{
    if (m_chunked_size == 0) {
        if (!m_is_chunked) {
            m_parse_step = ddhttp_parse_step::parse_trailer;
        } else {
            m_parse_step = ddhttp_parse_step::parse_chunked_end_line;
        }

        result.parse_state = dddata_parse_state::complete;
        return;
    }

    s32 need_size = m_chunked_size - m_body_recv_size;
    result.parsed_size = min(need_size, buff_size);
    m_context.body_buff = buff;
    m_context.body_buff_size = result.parsed_size;
    m_body_recv_size += result.parsed_size;

    if (m_chunked_size > m_body_recv_size) {
        result.parse_state = dddata_parse_state::need_more_data;
        return;
    }

    if (m_is_chunked) {
        // do not return complete but continue_parse to let the call get the body data.
        result.parse_state = dddata_parse_state::continue_parse;
        m_parse_step = ddhttp_parse_step::parse_chunked_end_line;
    } else {
        result.parse_state = dddata_parse_state::complete;
        m_parse_step = ddhttp_parse_step::parse_trailer;
    }
}

template <class T>
void ddhttp_parser<T>::parse_chunked_end_line_from_buff(u8* buff, s32 buff_size, dddata_parse_result& result)
{
    DDASSERT(m_is_chunked);
    std::string line;
    ddidata_parser::get_rnline_from_buff(buff, buff_size, line, result);
    if (result.parse_state != dddata_parse_state::complete) {
        // \r\n, 如果大于2个字符, 则认为是错误的chunked size
        if (buff_size >= 2) {
            result.parse_state = dddata_parse_state::error;
        }
        return;
    }

    if (m_chunked_size == 0) {
        // the last chunk
        m_parse_step = ddhttp_parse_step::parse_trailer;
    } else {
        // has more chunk
        m_parse_step = ddhttp_parse_step::parse_body_size;
    }
}

template <class T>
void ddhttp_parser<T>::parse_trailer_from_buff(u8*, s32, dddata_parse_result& result)
{
    m_parse_step = ddhttp_parse_step::parse_end;
    result.parse_state = dddata_parse_state::complete;
}

template class ddhttp_parser<ddhttp_request_header>;
template class ddhttp_parser<ddhttp_response_header>;
} // namespace NSP_DD