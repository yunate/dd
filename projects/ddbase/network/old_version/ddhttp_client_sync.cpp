#include "ddbase/stdafx.h"
#include "ddbase/network/ddhttp_client_sync.h"
#include "ddbase/pickle/ddidata_parser.h"
#include "ddbase/dderror_code.h"
#include "ddbase/ddassert.h"

#include <memory>
namespace NSP_DD {

std::unique_ptr<ddhttp_client_sync> ddhttp_client_sync::create_instance(std::shared_ptr<ddsocket_sync> socket)
{
    if (socket == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return nullptr;
    }

    std::unique_ptr<ddhttp_client_sync> client(new(std::nothrow) ddhttp_client_sync());
    if (client == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    client->m_socket = socket;
    client->m_buff.resize(2048);
    return client;
}

bool ddhttp_client_sync::send_header(const ddhttp_request_header& request_header)
{
    m_parser.reset();
    std::string request_str = request_header.to_str();
    u64 sended = m_socket->send((void*)request_str.data(), (u64)request_str.size());
    return (sended == (u64)request_str.size());
}

bool ddhttp_client_sync::send_body(const u8* buff, s32 buff_size)
{
    if (buff == nullptr || buff_size == 0) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return false;
    }
    u64 sended = m_socket->send((void*)buff, (u64)buff_size);
    return (sended == (u64)buff_size);
}

bool ddhttp_client_sync::send_body(ddistream* stream)
{
    if (stream == nullptr) {
        dderror_code::set_last_error(dderror_code::param_nullptr);
        return false;
    }

    u8 buff[1536];
    s64 sended_size = 0;
    while (stream->size() > sended_size) {
        s32 readed = stream->read(buff, sizeof(buff));
        if (readed == 0) {
            return false;
        }
        u64 sended = m_socket->send((void*)buff, (u64)readed);
        if (sended != (u64)readed) {
            return false;
        }
        sended_size += readed;
    }
    return true;
}

const ddhttp_response_parse::context* ddhttp_client_sync::recv()
{
    bool need_recv = true;
    if (m_parse_ctx != nullptr) {
        if (m_parse_ctx->parse_result.parsed_size > 0) {
            DDASSERT(m_parse_ctx->parse_result.parsed_size <= m_buff_remain_size);
            m_buff_remain_size -= m_parse_ctx->parse_result.parsed_size;
            if (m_buff_remain_size != 0) {
                (void)::memmove(m_buff.data(), m_buff.data() + m_parse_ctx->parse_result.parsed_size, m_buff_remain_size);
            }
        }

        if (m_parse_ctx->parse_result.parse_state != dddata_parse_state::need_more_data && m_buff_remain_size > 0) {
            need_recv = false;
        }
    }

    if (need_recv) {
        s32 recved = (s32)m_socket->recv(m_buff.data() + m_buff_remain_size, ((u64)m_buff.size() - m_buff_remain_size));
        if (recved == 0) {
            return nullptr;
        }
        m_buff_remain_size += (s32)(recved);
    }

    m_parser.continue_parse_from_buff(m_buff.data(), m_buff_remain_size);
    m_parse_ctx = m_parser.get_context_t();
    return m_parse_ctx;
}
} // namespace NSP_DD
