#ifndef ddbase_network_http_ddhttp_client_h_
#define ddbase_network_http_ddhttp_client_h_
#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include "ddbase/network/http/ddhttp_parser.h"
#include "ddbase/network/ddtcp.h"
#include "ddbase/stream/ddstream_view.h"
#include "ddbase/network/dddns_factory.h"
#include "ddbase/network/ddtls.h"
#include <memory>
////////////////////////////////////////////////////////ddhttp_client//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
class ddhttp_client
{
    DDNO_COPY_MOVE(ddhttp_client);
    ddhttp_client() = default;
public:
    static std::shared_ptr<ddhttp_client> create_inst(const ddasync_caller& async_caller = nullptr);
    void connect(ddiocp_with_dispatcher* iocp, const ddaddr& addr, const std::function<void(bool successful)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<bool> connect(ddiocp_with_dispatcher* iocp, const ddaddr& addr, ddexpire expire = ddexpire::never);

    void https_hand_shake(const std::string& host, const std::function<void(bool successful)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<bool> https_hand_shake(const std::string& host, ddexpire expire = ddexpire::never);

    void send_head(const ddhttp_request_header& head, const std::function<void(bool successful)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<bool> send_head(const ddhttp_request_header& head, ddexpire expire = ddexpire::never);

    // buff 在callback返回前不能被释放
    void send_body(void* buff, s32 buff_size, const std::function<void(bool successful)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<bool> send_body(void* buff, s32 buff_size, ddexpire expire = ddexpire::never);

    // stream 在callback返回false或者all_sended为true前不能被释放
    void send_stream(ddistream_view* stream, const std::function<void(bool successful, s32 byte, bool all_sended)>& callback, ddexpire expire = ddexpire::never);

    // head 为 nullptr 时候表示失败
    void recv_head(const std::function<void(const ddhttp_response_header* head)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<const ddhttp_response_header*> recv_head(ddexpire expire = ddexpire::never);

    struct recv_body_result
    {
        bool successful = false;
        bool end = false;
        const void* buff = nullptr;
        s32 buff_size = 0;
    };
    void recv_body(const std::function<void(const recv_body_result&)>& callback, ddexpire expire = ddexpire::never);
    ddcoroutine<recv_body_result> recv_body(ddexpire expire = ddexpire::never);

    ddtls* get_tls() const;
private:
    void send_inner(void* buff, s32 buff_size, ddexpire expire, const std::function<void(bool successful, s32 byte)>& callback);
    void recv_inner(void* buff, s32 buff_size, ddexpire expire, const std::function<void(bool successful, s32 byte)>& callback);
    void move_parser_buff(s32 parsed_size);
    ddasync_caller m_async_caller = nullptr;
    std::shared_ptr<ddtcp_connector> m_connector;
    std::shared_ptr<ddtcp_socket> m_socket;
    ddhttp_response_parser m_parser;
    std::vector<u8> m_parser_buff;
    s32 m_buff_remain_size = 0;

    // tls
    std::unique_ptr<ddtls> m_tls;

    // 接受的解密后的数据
    ddbuff m_recv_raw_buff;

    // weak
    std::weak_ptr<ddhttp_client> m_weak_this;
};
} // namespace NSP_DD

//////////////////////////////////////////////////////////ddhttp_requester//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
class ddhttp_requester
{
    ddhttp_requester() = default;
public:
    static std::shared_ptr<ddhttp_requester> make_get_requester(ddiocp_with_dispatcher* iocp, dddns_factory* dns_factory, const ddasync_caller& async_caller, const std::string& url);
    static std::shared_ptr<ddhttp_requester> make_post_requester(ddiocp_with_dispatcher* iocp, dddns_factory* dns_factory, const ddasync_caller& async_caller, const std::string& url, const std::string& body);

    ddcoroutine<const ddhttp_response_header*> recv_header(ddexpire expire = ddexpire::never);
    ddcoroutine<ddhttp_client::recv_body_result> recv_body(ddexpire expire = ddexpire::never);
private:
    ddiocp_with_dispatcher* m_iocp = nullptr;
    dddns_factory* m_dns_factory = nullptr;
    bool m_connected = false;
    std::shared_ptr<ddhttp_client> m_client;
    ddurl m_url;
    std::string m_body;
    std::string m_method;
    ddasync_caller m_async_caller = nullptr;
};
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_client_h_