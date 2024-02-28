#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/http/ddhttp_client.h"
#include "ddbase/ddexec_guard.hpp"
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

////////////////////////////////////////////////////////ddhttp_client//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
std::shared_ptr<ddhttp_client> ddhttp_client::create_inst(const ddasync_caller& async_caller /* = nullptr */)
{
    std::shared_ptr<ddhttp_client> inst(new(std::nothrow)ddhttp_client());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return  nullptr;
    }

    inst->m_weak_this = inst;
    inst->m_async_caller = async_caller;
    return inst;
}

void ddhttp_client::connect(ddiocp_with_dispatcher* iocp,
    const ddaddr& addr,
    const std::function<void(bool successful)>& callback,
    ddexpire expire /* = ddexpire::never*/)
{
    if (m_socket != nullptr) {
        callback(true);
        return;
    }

    if (m_connector == nullptr) {
        m_connector = ddtcp_connector::create_inst(iocp);
        if (m_connector == nullptr) {
            callback(false);
            return;
        }
    }

    m_connector->connect_to(addr, [this, callback](const std::shared_ptr<ddtcp_socket>& socket) {
        if (socket != nullptr) {
            m_socket = socket;
            m_parser_buff.resize(2048);
            callback(true);
        } else {
            callback(false);
        }
    }, expire);
}

ddcoroutine<bool> ddhttp_client::connect(ddiocp_with_dispatcher* iocp, const ddaddr& addr, ddexpire expire /* = ddexpire::never*/)
{
    bool return_value = false;
    co_await ddco_async([this, &return_value, iocp, &addr, expire](const ddresume_helper& resumer) {
        connect(iocp, addr, [&return_value, resumer](bool successful) {
            return_value = successful;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}

void ddhttp_client::https_hand_shake(const std::string& host, const std::function<void(bool successful)>& callback, ddexpire expire /* = ddexpire::never*/)
{
    if (m_tls == nullptr) {
        m_tls = ddtls::create_client(host);
        if (m_tls == nullptr) {
            callback(false);
            return;
        }
    }

    ddtcp_socket* socket = m_socket.get();
    auto read_opt = [socket](void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, ddexpire expire) {
        socket->read(buff, buff_size, callback, expire);
    };

    auto write_opt = [socket](void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, ddexpire expire) {
        socket->write(buff, buff_size, callback, expire);
    };

    m_tls->handshake(read_opt, write_opt, callback, m_async_caller, expire);

}

ddcoroutine<bool> ddhttp_client::https_hand_shake(const std::string& host, ddexpire expire /* = ddexpire::never*/)
{
    bool return_value = false;
    co_await ddco_async([this, &return_value, &host, expire](const ddresume_helper& resumer) {
        https_hand_shake(host, [&return_value, resumer](bool successful) {
            return_value = successful;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}

void ddhttp_client::send_inner(void* buff, s32 buff_size, ddexpire expire, const std::function<void(bool successful, s32 byte)>& callback)
{
    DDASSERT(buff != nullptr);
    DDASSERT_FMTW(m_socket != nullptr, L"should call connect before call this function.");
    if (m_tls != nullptr) {
        ddmaybe_async_call(m_async_caller, [=, weak = m_weak_this]() {
            auto sp = weak.lock();
            if (sp == nullptr) {
                callback(false, 0);
                return;
            }

            auto result = m_tls->encode((u8*)buff, buff_size);
            if (!result.successful) {
                callback(false, 0);
                return;
            }

            m_socket->write((u8*)result.buff, result.buff_size, [buff_size, callback](bool successful, s32) {
                // we think the buff had been all sent, although some data was buffed in m_tls.
                callback(successful, buff_size);
            }, expire);
        });
    } else {
        m_socket->write(buff, buff_size, callback, expire);
    }
}

void ddhttp_client::send_head(const ddhttp_request_header& head, const std::function<void(bool successful)>& callback, ddexpire expire /* = ddexpire::never*/)
{
    std::shared_ptr<std::string> head_str = std::make_shared<std::string>();
    *head_str = head.to_str();
    send_inner((*head_str).data(), (s32)((*head_str).size()), expire, [callback, head_str](bool successful, s32) {
        callback(successful);
    });
}

ddcoroutine<bool> ddhttp_client::send_head(const ddhttp_request_header& head, ddexpire expire /* = ddexpire::never*/)
{
    bool return_value = false;
    co_await ddco_async([this, &return_value, &head, expire](const ddresume_helper& resumer) {
        send_head(head, [&return_value, resumer](bool successful) {
            return_value = successful;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}

void ddhttp_client::send_body(void* buff, s32 buff_size, const std::function<void(bool successful)>& callback, ddexpire expire /* = ddexpire::never*/)
{
    send_inner(buff, buff_size, expire, [callback](bool successful, s32) {
        callback(successful);
    });
}

ddcoroutine<bool> ddhttp_client::send_body(void* buff, s32 buff_size, ddexpire expire /* = ddexpire::never*/)
{
    bool return_value = false;
    co_await ddco_async([this, &return_value, buff, buff_size, expire](const ddresume_helper& resumer) {
        send_body(buff, buff_size, [&return_value, resumer](bool successful) {
            return_value = successful;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}

void ddhttp_client::send_stream(ddistream_view* stream, const std::function<void(bool successful, s32 byte, bool all_sended)>& callback, ddexpire expire /* = ddexpire::never*/)
{
    DDASSERT(stream != nullptr);
    DDASSERT_FMTW(m_socket != nullptr, L"should call connect before call this function.");
    m_socket->write(stream, callback, m_async_caller, expire);
}

void ddhttp_client::recv_inner(void* buff, s32 buff_size, ddexpire expire, const std::function<void(bool successful, s32 byte)>& callback)
{
    if (m_tls != nullptr) {
        if (!m_recv_raw_buff.empty()) {
            s32 cpy_size = min(buff_size, (s32)m_recv_raw_buff.size());
            ::memcpy(buff, m_recv_raw_buff.data(), cpy_size);
            remain(m_recv_raw_buff, (s32)m_recv_raw_buff.size() - cpy_size);
            callback(true, cpy_size);
            return;
        }

        m_socket->read(buff, buff_size, [callback, this, buff, buff_size](bool successful, s32 byte) {
            if (!successful) {
                callback(false, 0);
                return;
            }

            ddmaybe_async_call(m_async_caller, [=, weak = m_weak_this]() {
                auto sp = weak.lock();
                if (sp == nullptr) {
                    callback(false, 0);
                    return;
                }

                auto result = m_tls->decode((u8*)buff, byte);
                if (!result.successful) {
                    callback(false, 0);
                    return;
                }

                push_back(m_recv_raw_buff, (u8*)result.buff, result.buff_size);
                s32 cpy_size = min(buff_size, (s32)m_recv_raw_buff.size());
                ::memcpy(buff, m_recv_raw_buff.data(), cpy_size);
                remain(m_recv_raw_buff, (s32)m_recv_raw_buff.size() - cpy_size);
                callback(true, cpy_size);
            });
        }, expire);
    } else {
        m_socket->read(buff, buff_size, callback, expire);
    }
}

void ddhttp_client::recv_head(const std::function<void(const ddhttp_response_header* head)>& callback, ddexpire expire /* = ddexpire::never*/)
{
    ddmaybe_async_call(m_async_caller, [=, weak = m_weak_this]() {
        auto sp = weak.lock();
        if (sp == nullptr) {
            callback(nullptr);
            return;
        }
        DDASSERT_FMTW(m_socket != nullptr, L"should call connect before call this function.");
        auto ctx = m_parser.get_context_t();
        DDASSERT(ctx != nullptr);

        if (ctx->head_parsed) {
            callback(&(ctx->header));
            return;
        }

        if (ctx->parse_result.parse_state == dddata_parse_state::error) {
            callback(nullptr);
            return;
        } else if (ctx->parse_result.parse_state == dddata_parse_state::complete) {
            callback(&(ctx->header));
            return;
        } else if (ctx->parse_result.parse_state == dddata_parse_state::continue_parse) {
            move_parser_buff(ctx->parse_result.parsed_size);
            (void)m_parser.continue_parse_from_buff(m_parser_buff.data(), m_buff_remain_size);
            recv_head(callback, expire);
            return;
        } else if (ctx->parse_result.parse_state == dddata_parse_state::need_more_data) {
            move_parser_buff(ctx->parse_result.parsed_size);
            if (m_buff_remain_size == (s32)m_parser_buff.size()) {
                callback(nullptr);
                return;
            }
            recv_inner(m_parser_buff.data() + m_buff_remain_size, (s32)m_parser_buff.size() - m_buff_remain_size, expire, [this, callback, expire](bool successful, s32 byte) {
                if (successful) {
                    m_buff_remain_size += byte;
                    (void)m_parser.continue_parse_from_buff(m_parser_buff.data(), m_buff_remain_size);
                    recv_head(callback, expire);
                } else {
                    callback(nullptr);
                }
            });
        }
    });
}

ddcoroutine<const ddhttp_response_header*> ddhttp_client::recv_head(ddexpire expire /* = ddexpire::never*/)
{
    const ddhttp_response_header* return_value = nullptr;
    co_await ddco_async([this, &return_value, expire](const ddresume_helper& resumer) {
        recv_head([&return_value, resumer](const ddhttp_response_header* head) {
            return_value = head;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}

void ddhttp_client::recv_body(const std::function<void(const recv_body_result&)>& callback, ddexpire expire /* = ddexpire::never*/)
{
    ddmaybe_async_call(m_async_caller, [=, weak = m_weak_this]() {
        auto sp = weak.lock();
        if (sp == nullptr) {
            recv_body_result result;
            result.successful = false;
            callback(result);
            return;
        }

        DDASSERT_FMTW(m_socket != nullptr, L"should call connect before call this function.");
        auto ctx = m_parser.get_context_t();
        DDASSERT(ctx != nullptr);

        if (ctx->parse_result.parse_state == dddata_parse_state::error) {
            recv_body_result result;
            result.successful = false;
            callback(result);
            return;
        }
 
        if (ctx->parse_result.parse_state == dddata_parse_state::complete) {
            recv_body_result result;
            result.successful = true;
            result.end = true;
            result.buff = ctx->body_buff;
            result.buff_size = ctx->body_buff_size;
            ctx->body_buff = nullptr;
            ctx->body_buff_size = 0;
            callback(result);
            return;
        }

        if (ctx->body_buff_size != 0) {
            recv_body_result result;
            result.successful = true;
            result.end = false;
            result.buff = ctx->body_buff;
            result.buff_size = ctx->body_buff_size;
            ctx->body_buff = nullptr;
            ctx->body_buff_size = 0;
            callback(result);
            return;
        }

        if (ctx->parse_result.parse_state == dddata_parse_state::continue_parse) {
            move_parser_buff(ctx->parse_result.parsed_size);
            (void)m_parser.continue_parse_from_buff(m_parser_buff.data(), m_buff_remain_size);
            recv_body(callback, expire);
            return;
        }
        
        if (ctx->parse_result.parse_state == dddata_parse_state::need_more_data) {
            move_parser_buff(ctx->parse_result.parsed_size);
            if (m_buff_remain_size == (s32)m_parser_buff.size()) {
                recv_body_result result;
                result.successful = false;
                callback(result);
                return;
            }
            recv_inner(m_parser_buff.data() + m_buff_remain_size, (s32)m_parser_buff.size() - m_buff_remain_size, expire, [this, callback, expire](bool successful, s32 byte) {
                if (successful) {
                    m_buff_remain_size += byte;
                    (void)m_parser.continue_parse_from_buff(m_parser_buff.data(), m_buff_remain_size);
                    recv_body(callback, expire);
                } else {
                    recv_body_result result;
                    result.successful = false;
                    callback(result);
                }
            });
        }
    });
}

ddcoroutine<ddhttp_client::recv_body_result> ddhttp_client::recv_body(ddexpire expire /* = ddexpire::never*/)
{
    recv_body_result return_value;
    co_await ddco_async([this, &return_value, expire](const ddresume_helper& resumer) {
        recv_body([&return_value, resumer](const recv_body_result& result) {
            return_value = result;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}

ddtls* ddhttp_client::get_tls() const
{
    return m_tls.get();
}

void ddhttp_client::move_parser_buff(s32 parsed_size)
{
    if (parsed_size > 0) {
        DDASSERT(parsed_size <= m_buff_remain_size);
        m_buff_remain_size -= parsed_size;
        if (m_buff_remain_size != 0) {
            (void)::memmove(m_parser_buff.data(), m_parser_buff.data() + parsed_size, m_buff_remain_size);
        }
    }
}
} // namespace NSP_DD

////////////////////////////////////////////////////////ddhttp_requester//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
std::shared_ptr<ddhttp_requester> ddhttp_requester::make_get_requester(ddiocp_with_dispatcher* iocp, dddns_factory* dns_factory, const ddasync_caller& async_caller, const std::string& url)
{
    std::shared_ptr<ddhttp_requester> inst(new (std::nothrow)ddhttp_requester());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    DDASSERT(iocp != nullptr);
    inst->m_iocp = iocp;
    DDASSERT(dns_factory != nullptr);
    inst->m_dns_factory = dns_factory;

    parse_url(url, inst->m_url);
    if (!inst->m_url.is_valid()) {
        dderror_code::set_last_error(dderror_code::invalid_url);
        return nullptr;
    }

    inst->m_async_caller = async_caller;
    inst->m_method = "GET";
    return inst;
}

std::shared_ptr<ddhttp_requester> ddhttp_requester::make_post_requester(ddiocp_with_dispatcher* iocp, dddns_factory* dns_factory, const ddasync_caller& async_caller, const std::string& url, const std::string& body)
{
    std::shared_ptr<ddhttp_requester> inst = ddhttp_requester::make_get_requester(iocp, dns_factory, async_caller, url);
    if (inst == nullptr) {
        return nullptr;
    }

    inst->m_body = body;
    inst->m_method = "POST";
    return inst;
}

static ddcoroutine<std::pair<std::string, bool>> get_ip(dddns_factory* factory, const ddasync_caller& async_caller, const ddurl& url)
{
    std::pair<std::string, bool> return_value;
    co_await ddco_async([&return_value, factory, &url, &async_caller](const ddresume_helper& resumer) {
        ddmaybe_async_call(async_caller, [resumer, &return_value, factory, &url]() {
            auto dns_entry = factory->get_ip(url.host);
            if (dns_entry != nullptr) {
                if (!dns_entry->ip_v4s.empty()) {
                    return_value.first = dns_entry->ip_v4s[0];
                    return_value.second = true;
                } else if (!dns_entry->ip_v6s.empty()) {
                    return_value.first = dns_entry->ip_v6s[0];
                    return_value.second = false;
                }
            }

            resumer.lazy_resume();
        });
    });
    co_return return_value;
}

ddcoroutine<const ddhttp_response_header*> ddhttp_requester::recv_header(ddexpire expire /* = ddexpire::never */)
{
    if (!m_connected) {
        if (m_client == nullptr) {
            m_client = ddhttp_client::create_inst(m_async_caller);
            if (m_client == nullptr) {
                co_return false;
            }
        }

        bool use_tls = (ddstr::lower(m_url.scheme.c_str()) == "https");
        ddaddr addr;
        addr.port = use_tls ? 443 : 80;
        std::pair<std::string, bool> ip_pair = co_await get_ip(m_dns_factory, m_async_caller, m_url);
        addr.ip = ip_pair.first;
        addr.ipv4_6 = ip_pair.second;
        if (addr.ip.empty()) {
            dderror_code::set_last_error(dderror_code::invalid_url);
            co_return nullptr;
        }

        m_connected = co_await m_client->connect(m_iocp, addr, expire);
        if (!m_connected) {
            co_return nullptr;
        }

        if (use_tls) {
            if (!co_await m_client->https_hand_shake(m_url.host, expire)) {
                co_return nullptr;
            }
        }
    }

    ddhttp_request_header request_header;
    request_header.as_default(m_url);
    if (!m_body.empty()) {
        request_header.kvs.set_content_lenth((s32)m_body.size());
    }

    if (!co_await m_client->send_head(request_header, expire)) {
        co_return nullptr;
    }

    if (!m_body.empty()) {
        if (!co_await m_client->send_body(m_body.data(), (s32)m_body.size(), expire)) {
            co_return nullptr;
        }
    }

    co_return co_await m_client->recv_head(expire);
}

ddcoroutine<ddhttp_client::recv_body_result> ddhttp_requester::recv_body(ddexpire expire /* = ddexpire::never */)
{
    DDASSERT(m_client != nullptr);
    co_return co_await m_client->recv_body(expire);
}
} // namespace NSP_DD

