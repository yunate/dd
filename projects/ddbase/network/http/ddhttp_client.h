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
    static std::shared_ptr<ddhttp_client> create_inst(ddiocp_with_dispatcher* iocp, const std::shared_ptr<dddns_factory>& dns_factory, const ddasync_caller& async_caller = nullptr);
    ~ddhttp_client();
    void close_socket();
    ddcoroutine<std::shared_ptr<ddhttp_client_socket>> connect_to(const ddurl& url, bool force_https, ddexpire expire = ddexpire::never);

private:
    std::shared_ptr<ddtcp_connector> m_connector;
    std::shared_ptr<dddns_factory> m_dns_factory;
    ddasync_caller m_async_caller = nullptr;
};
} // namespace NSP_DD

//////////////////////////////////////////////////////////ddhttp_requester//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
class ddhttp_requester
{
    ddhttp_requester() = default;
public:
    static ddcoroutine<std::shared_ptr<ddhttp_requester>> create_inst(const std::shared_ptr<ddhttp_client>& client,
        const ddurl& url,
        bool force_https,
        ddexpire expire);
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
    ddcoroutine<const ddhttp_response_header*> post(const std::string& body, ddexpire expire);
    ddcoroutine<const ddhttp_response_header*> get(const std::string& uri, ddexpire expire);
    ddcoroutine<ddhttp_recv_body_result> recv_body(ddexpire expire);

    // 用来清理缓存, 如果上次请求还没有完成，返回false
    bool reset();

private:
    ddcoroutine<const ddhttp_response_header*> send_and_get_header(const std::string& method,
        const std::string& uri, const std::string& body,
        ddexpire expire);

    std::shared_ptr<ddhttp_client_socket> m_client_item;
    ddurl m_url;
    ddasync_caller m_async_caller = nullptr;
    bool m_pre_opt_completed = true;
};
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_client_h_