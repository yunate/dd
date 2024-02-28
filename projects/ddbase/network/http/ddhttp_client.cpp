#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"

#include "ddbase/network/http/ddhttp_client.h"
#include "ddbase/ddexec_guard.hpp"
#include <mswsock.h>

////////////////////////////////////////////////////////ddhttp_client//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
std::shared_ptr<ddhttp_client> ddhttp_client::create_inst(ddiocp_with_dispatcher* iocp)
{
    std::shared_ptr<ddhttp_client> inst(new(std::nothrow)ddhttp_client());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    inst->m_connector = ddtcp_connector::create_inst(iocp);
    if (inst->m_connector == nullptr) {
        return nullptr;
    }

    return inst;
}

ddhttp_client::~ddhttp_client()
{
    close_socket();
}

void ddhttp_client::close_socket()
{
    if (m_connector != nullptr) {
        m_connector->close_socket();
        m_connector.reset();
    }
}

class ddhttp_socket_client_helper : public ddhttp_client_socket
{
public:
    void set_tcp_socket(const std::shared_ptr<ddtcp_socket>& socket)
    {
        m_socket = socket;
    }
};

ddcoroutine<std::shared_ptr<ddhttp_client_socket>> ddhttp_client::connect_to(const ddaddr& addr, const std::string& host, bool force_https, ddexpire expire /* = ddexpire::never*/)
{
    auto tcp_socket = co_await m_connector->connect_to(addr, expire);
    if (tcp_socket == nullptr) {
        co_return nullptr;
    }

    std::shared_ptr<ddhttp_socket_client_helper> client = std::make_shared<ddhttp_socket_client_helper>();
    if (client == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        co_return nullptr;
    }

    client->set_tcp_socket(tcp_socket);

    if (force_https || addr.port == 443) {
        if (!client->init_tls(host) || !co_await client->https_hand_shake(expire)) {
            co_return nullptr;
        }
    }

    co_return client;
}
} // namespace NSP_DD

////////////////////////////////////////////////////////ddhttp_requester//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
std::shared_ptr<ddhttp_requester> ddhttp_requester::create_inst(const std::shared_ptr<ddhttp_client_socket>& socket, const std::string& host)
{
    std::shared_ptr<ddhttp_requester> inst(new (std::nothrow)ddhttp_requester());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    DDASSERT(socket != nullptr);
    inst->m_client_item = socket;
    inst->m_host = host;
    return inst;
}

ddhttp_requester::~ddhttp_requester()
{
    close_socket();
}

void ddhttp_requester::close_socket()
{
    if (m_client_item != nullptr) {
        m_client_item->close_socket();
        m_client_item.reset();
    }
}

ddcoroutine<const ddhttp_response_header*> ddhttp_requester::post(const std::string& body, ddexpire expire)
{
   co_return co_await send_and_get_header("POST", "", body, expire);
}

ddcoroutine<const ddhttp_response_header*> ddhttp_requester::get(const std::string& uri, ddexpire expire)
{
    co_return co_await send_and_get_header("GET", uri, "", expire);
}

ddcoroutine<const ddhttp_response_header*> ddhttp_requester::send_and_get_header(const std::string& method,
    const std::string& uri, const std::string& body,
    ddexpire expire) {
    if (!reset()) {
        co_return nullptr;
    }

    const ddhttp_response_header* head = nullptr;
    do {
        ddhttp_request_header request_header;
        request_header.as_default(m_host);
        request_header.method = method;

        if (uri.empty()) {
            request_header.uri = "/";
        } else {
            if (uri[0] != '/') {
                request_header.uri += "/";
            }
            request_header.uri += uri;
        }

        if (!body.empty()) {
            request_header.kvs.set_content_lenth((s32)body.size());
        }

        if (!co_await m_client_item->send_head(request_header, expire)) {
            break;
        }

        if (!body.empty()) {
            if (!co_await m_client_item->send_body((void*)body.data(), (s32)body.size(), expire)) {
                break;
            }
        }

        co_return co_await m_client_item->recv_head(expire);
    } while (0);

    m_pre_opt_completed = true;
    if (head != nullptr && head->kvs.content_lenth() != 0) {
        // we still have some body data to recv.
        m_pre_opt_completed = false;
    }
    co_return head;
}

ddcoroutine<ddhttp_recv_body_result> ddhttp_requester::recv_body(ddexpire expire)
{
    DDASSERT(m_client_item != nullptr);
    m_pre_opt_completed = true;

    ddhttp_recv_body_result result = co_await m_client_item->recv_body(expire);
    if (!result.successful || result.end) {
        m_pre_opt_completed = true;
    }
    co_return result;
}

bool ddhttp_requester::reset()
{
    if (!m_pre_opt_completed) {
        return false;
    }
    DDASSERT(m_client_item != nullptr);
    m_client_item->reset();
    m_pre_opt_completed = false;
    return true;
}
} // namespace NSP_DD

