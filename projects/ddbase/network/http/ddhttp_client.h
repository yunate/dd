#ifndef ddbase_network_http_ddhttp_client_h_
#define ddbase_network_http_ddhttp_client_h_
#include "ddbase/dddef.h"
#include "ddbase/network/http/ddhttp_socket.h"

////////////////////////////////////////////////////////ddhttp_client//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
class ddhttp_client
{
    DDNO_COPY_MOVE(ddhttp_client);
    ddhttp_client() = default;
public:
    static std::shared_ptr<ddhttp_client> create_inst(ddiocp_with_dispatcher* iocp);
    ~ddhttp_client();
    void close_socket();
    // tls_host: www.XXX.com; 可以为空字符串, 当其为空字符串时候表示不使用tls
    ddcoroutine<std::shared_ptr<ddhttp_client_socket>> connect_to(const ddaddr& addr, DDCO_REF const std::string& tls_host, ddexpire expire = ddexpire::never);

private:
    std::shared_ptr<ddtcp_connector> m_connector;
};
} // namespace NSP_DD

//////////////////////////////////////////////////////////ddhttp_requester//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
class ddhttp_requester
{
    ddhttp_requester() = default;
public:
    static std::shared_ptr<ddhttp_requester> create_inst(const std::shared_ptr<ddhttp_client_socket>& socket, const std::string& host);
    ~ddhttp_requester();
    void close_socket();
    inline SOCKET get_socket()
    {
        if (m_client_item == nullptr) {
            return INVALID_SOCKET;
        }
        return m_client_item->get_socket();
    }

    // 如果上次的请求还没有完成比如还有数据没有接受完毕，返回nullptr
    // 可以使用reset函数来判断是否可以继续请求
    ddcoroutine<const ddhttp_response_header*> post(DDCO_REF const std::string& body, ddexpire expire);
    ddcoroutine<const ddhttp_response_header*> get(const std::string& uri, ddexpire expire);
    ddcoroutine<ddhttp_recv_body_result> recv_body(ddexpire expire);

    // 用来清理缓存, 如果上次请求还没有完成，返回false
    bool reset();

private:
    ddcoroutine<const ddhttp_response_header*> send_and_get_header(const std::string& method,
        const std::string& uri, DDCO_REF const std::string& body,
        ddexpire expire);

    std::shared_ptr<ddhttp_client_socket> m_client_item;
    std::string m_host;
    bool m_pre_opt_completed = true;
};
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_client_h_