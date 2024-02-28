#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/http/ddhttp_socket.h"
#include <mswsock.h>
namespace NSP_DD {
static void remain(ddbuff& src, s32 remain_size)
{
    ::memcpy(src.data(), src.data() + src.size() - remain_size, remain_size);
    src.resize(remain_size);
}

static void push_back(ddbuff& src, void* buff, s32 buff_size)
{
    src.resize(src.size() + buff_size);
    ::memcpy(src.data() + src.size() - buff_size, buff, buff_size);
}
} // namespace NSP_DD

namespace NSP_DD {
#define DDHTTP_SOCKET ddhttp_socket<SNED_HEADER, RECV_HEADER>

template <class SNED_HEADER, class RECV_HEADER>
DDHTTP_SOCKET::ddhttp_socket() : m_parser([this](void* buff, s32 buff_size, ddexpire expire) ->ddcoroutine<std::tuple<bool, s32>> {
    co_return co_await this->recv_inner(buff, buff_size, expire);
})
{ }

template <class SNED_HEADER, class RECV_HEADER>
DDHTTP_SOCKET::~ddhttp_socket()
{
    close_socket();
}

template <class SNED_HEADER, class RECV_HEADER>
void DDHTTP_SOCKET::close_socket()
{
    if (m_socket != nullptr) {
        m_socket->close_socket();
        m_socket.reset();
    }
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<std::tuple<bool, s32>> DDHTTP_SOCKET::send_inner(void* buff, s32 buff_size, ddexpire expire)
{
    DDASSERT(buff != nullptr);
    DDASSERT_FMTW(m_socket != nullptr, L"should call connect before call this function.");
    if (m_tls != nullptr) {
        auto encode_result = m_tls->encode((u8*)buff, buff_size);
        if (!encode_result.successful) {
            co_return{ false, 0 };
        }

        auto write_result = co_await m_socket->write((u8*)encode_result.buff, encode_result.buff_size, expire);

        // we think the buff had been all sent, although some data was buffed in m_tls.
        co_return{ std::get<0>(write_result), buff_size };
    } else {
        co_return co_await m_socket->write(buff, buff_size, expire);
    }
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<bool> DDHTTP_SOCKET::send_head(const SNED_HEADER& head, ddexpire expire /* = ddexpire::never*/)
{
    std::shared_ptr<std::string> head_str = std::make_shared<std::string>();
    *head_str = head.to_str();
    auto result = co_await send_inner((*head_str).data(), (s32)((*head_str).size()), expire);
    co_return std::get<0>(result);
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<bool> DDHTTP_SOCKET::send_body(void* buff, s32 buff_size, ddexpire expire /* = ddexpire::never*/)
{
    auto result = co_await send_inner(buff, buff_size, expire);
    co_return std::get<0>(result);
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<std::tuple<bool, s32>> DDHTTP_SOCKET::recv_inner(void* buff, s32 buff_size, ddexpire expire)
{
    if (m_tls != nullptr) {
        while (true) {
            if (!m_recv_decoded_buff.empty()) {
                s32 cpy_size = min(buff_size, (s32)m_recv_decoded_buff.size());
                ::memcpy(buff, m_recv_decoded_buff.data(), cpy_size);
                remain(m_recv_decoded_buff, (s32)m_recv_decoded_buff.size() - cpy_size);
                co_return{ true, cpy_size };
            }

            auto read_result = co_await m_socket->read(buff, buff_size, expire);
            bool successful = std::get<0>(read_result);
            s32 byte = std::get<1>(read_result);
            if (!successful) {
                co_return{ false, 0 };
            }
            auto decode_result = m_tls->decode((u8*)buff, byte);
            if (!decode_result.successful) {
                co_return{ false, 0 };
            }

            push_back(m_recv_decoded_buff, (u8*)decode_result.buff, decode_result.buff_size);
        }
    } else {
        co_return co_await m_socket->read(buff, buff_size, expire);
    }
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<const RECV_HEADER*> DDHTTP_SOCKET::recv_head(ddexpire expire /* = ddexpire::never*/)
{
    DDASSERT_FMTA(m_parser.pre_parse_complete(), "use recv_to_end or recv_body to recv all the data before recv another head.");
    while (true) {
        auto state = co_await m_parser.continue_parse(expire);
        if (state == dddata_parse_state::error) {
            break;
        } else {
            auto ctx = m_parser.get_context_t();
            if (ctx->head_parsed) {
                co_return &ctx->header;
            }
        }
    }

    reset();
    co_return nullptr;
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<ddhttp_recv_body_result> DDHTTP_SOCKET::recv_body(ddexpire expire /* = ddexpire::never*/)
{
    ddhttp_recv_body_result result;
    while (true) {
        auto state = co_await m_parser.continue_parse(expire);
        if (state == dddata_parse_state::error) {
            break;
        } else {
            auto ctx = m_parser.get_context_t();
            result.successful = true;
            result.buff = ctx->body_buff;
            result.buff_size = ctx->body_buff_size;
            result.end = (state == dddata_parse_state::complete);
            co_return result;
        }
    }

    reset();
    result.successful = false;
    co_return result;
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<bool> DDHTTP_SOCKET::recv_to_end(ddexpire expire /* = ddexpire::never*/)
{
    ddhttp_recv_body_result result;
    while (true) {
        auto state = co_await m_parser.continue_parse(expire);
        if (state == dddata_parse_state::error) {
            co_return false;
        } else if (state == dddata_parse_state::complete) {
            co_return true;
        }
    }
    co_return false;
}

template <class SNED_HEADER, class RECV_HEADER>
ddtls* DDHTTP_SOCKET::get_tls() const
{
    return m_tls.get();
}

template <class SNED_HEADER, class RECV_HEADER>
void DDHTTP_SOCKET::reset()
{
    m_parser.reset();
    m_recv_decoded_buff.resize(0);
}

template <class SNED_HEADER, class RECV_HEADER>
ddcoroutine<bool> DDHTTP_SOCKET::https_hand_shake(ddexpire expire /* = ddexpire::never*/)
{
    DDASSERT(m_tls != nullptr);
    ddtcp_socket* socket = m_socket.get();
    auto read_opt = [socket](void* buff, s32 buff_size, ddexpire expire) -> ddcoroutine<std::tuple<bool, s32>> {
        co_return co_await socket->read(buff, buff_size, expire);
    };

    auto write_opt = [socket](void* buff, s32 buff_size, ddexpire expire) -> ddcoroutine<std::tuple<bool, s32>> {
        co_return co_await socket->write(buff, buff_size, expire);
    };

    co_return co_await m_tls->handshake(read_opt, write_opt, expire);
}

template class ddhttp_socket<ddhttp_request_header, ddhttp_response_header>;
ddcoroutine<bool> ddhttp_client_socket::init(const std::shared_ptr<ddtcp_socket>& socket, const std::string& tls_host, ddexpire expire)
{
    m_socket = socket;

    if (tls_host.empty()) {
        co_return true;
    }

    m_tls = ddtls::create_client(tls_host);
    if (m_tls == nullptr) {
        co_return false;
    }
    co_return co_await https_hand_shake(expire);
}

template class ddhttp_socket<ddhttp_response_header, ddhttp_request_header>;
ddcoroutine<bool> ddhttp_server_socket::init(const std::shared_ptr<ddtcp_socket>& socket, const std::shared_ptr<ddcert>& cert, ddexpire expire)
{
    m_socket = socket;

    if (cert == nullptr) {
        co_return true;
    }

    m_tls = ddtls::create_server(cert);
    if (m_tls == nullptr) {
        co_return false;
    }
    co_return co_await https_hand_shake(expire);
}
} // namespace NSP_DD

