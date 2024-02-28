#ifndef ddbase_network_http_ddhttp_server_h_
#define ddbase_network_http_ddhttp_server_h_
#include "ddbase/dddef.h"
#include "ddbase/network/http/ddhttp_socket.h"
#include "ddbase/network/ddcert.h"

namespace NSP_DD {
class ddhttp_server
{
    DDNO_COPY_MOVE(ddhttp_server);
    ddhttp_server() = default;
public:
    // cert可以为nullptr, 表示不使用https
    static std::shared_ptr<ddhttp_server> create_inst(
        ddiocp_with_dispatcher* iocp,
        const ddaddr& listen_addr,
        const std::shared_ptr<ddcert>& cert);
    ~ddhttp_server();
    void close_socket();
    inline SOCKET get_socket()
    {
        if (m_acceptor == nullptr) {
            return INVALID_SOCKET;
        }
        return m_acceptor->get_socket();
    }

    ddcoroutine<std::shared_ptr<ddhttp_server_socket>> accept(ddexpire expire = ddexpire::never);

private:
    std::shared_ptr<ddtcp_acceptor> m_acceptor = nullptr;
    std::shared_ptr<ddcert> m_cert;
};
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_server_h_