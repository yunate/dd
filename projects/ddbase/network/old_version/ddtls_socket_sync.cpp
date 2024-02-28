#include "ddbase/stdafx.h"
#include "ddbase/network/ddtls_socket_sync.h"
#include "ddbase/ddassert.h"
#include <algorithm>
namespace NSP_DD {

ddtls_socket_sync::ddtls_socket_sync(const std::shared_ptr<ddtls>& tls, const std::shared_ptr<ddsocket_sync>& socket) :
    m_tls(tls), m_socket(socket)
{
    DDASSERT(m_tls != nullptr);
    DDASSERT(m_socket != nullptr);
}

u64 ddtls_socket_sync::send(void* buff, u64 buff_size)
{
    std::vector<u8> out_buff;
    out_buff.resize(min(buff_size, m_tls->get_tls_encode_max_size()) + m_tls->get_tls_encode_extra_size());
    s32 size = (s32)buff_size;
    while (size > 0) {
        s32 out_buff_size = (s32)out_buff.size();
        if (!m_tls->encode((u8*)buff + (s32)buff_size - size, size, out_buff.data(), out_buff_size)) {
            return 0;
        }

        if (m_socket->send(out_buff.data(), out_buff_size) == 0) {
            return 0;
        }
    }
    return buff_size;
}

u64 ddtls_socket_sync::recv(void* buff, u64 buff_size)
{
    while (true) {
        if (m_decoded_size > 0) {
            if (m_decoded_size <= buff_size) {
                ::memmove(buff, m_buff.data() + m_decoded_data_begin_pos, m_decoded_size);
                s32 return_size = m_decoded_size;
                m_decoded_size = 0;
                if (m_encoded_size > 0) {
                    ::memmove(m_buff.data(), m_buff.data() + m_encoded_data_begin_pos, m_encoded_size);
                    m_encoded_data_begin_pos = 0;
                } else {
                    m_buff.resize(0);
                    m_buff.shrink_to_fit();
                }
                m_decoded_data_begin_pos = 0;
                return return_size;
            } else {
                ::memmove(buff, m_buff.data() + m_decoded_data_begin_pos, (size_t)buff_size);
                m_decoded_size -= (s32)buff_size;
                m_decoded_data_begin_pos += (s32)buff_size;
                return buff_size;
            }
        }

        if (m_encoded_size == 0) {
            m_buff.resize((s64)m_tls->get_tls_encode_max_size() + m_tls->get_tls_encode_extra_size());
        }

        if (m_encoded_size == (s32)m_buff.size()) {
            break;
        }

        s32 current_recved = (s32)m_socket->recv(m_buff.data() + m_encoded_size, (u64)m_buff.size() - m_encoded_size);
        if (current_recved == 0) {
            break;
        }

        m_encoded_size += current_recved;
        s32 raw_size = m_encoded_size;
        if (!m_tls->decode(m_buff.data(), m_encoded_size, m_decoded_size)) {
            break;
        }
        m_decoded_data_begin_pos = 0;
        m_encoded_data_begin_pos = raw_size - m_encoded_size;
    }

    m_buff.resize(0);
    m_buff.shrink_to_fit();
    return 0;
}

bool ddtls_socket_sync::hand_shake()
{
    s32 buff_size = m_tls->get_hand_shake_buff_size();
    std::vector<u8> in_buff(buff_size);
    std::vector<u8> out_buff(buff_size);
    s32 in_buff_size = buff_size;
    s32 out_buff_size = buff_size;

    while (true) {
        ddtls_handshake_ctx* tls_ctx = m_tls->get_handshanke_ctx();

        if (tls_ctx->need_send) {
            tls_ctx->need_send = false;

            s32 remain = out_buff_size;
            while (remain > 0) {
                s32 current_sended = (s32)m_socket->send(out_buff.data() + out_buff_size - remain, remain);
                if (current_sended == 0) {
                    return false;
                }
                remain -= current_sended;
            }
        }

        if (tls_ctx->need_recv) {
            tls_ctx->need_recv = false;
            in_buff_size += (s32)m_socket->recv(in_buff.data() + in_buff_size, (s64)buff_size - in_buff_size);
        }

        if (tls_ctx->complete) {
            tls_ctx->complete = false;
            return true;
        }

        out_buff_size = (s32)out_buff.size();
        if (!m_tls->continue_handshake(in_buff.data(), in_buff_size, out_buff.data(), out_buff_size)) {
            return false;
        }

        if (in_buff_size == buff_size) {
            return false;
        }
    }
    return false;
}

} // namespace NSP_DD
