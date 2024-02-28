#ifndef ddbase_network_http_ddhttp_socket_h_
#define ddbase_network_http_ddhttp_socket_h_

#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include "ddbase/network/http/ddhttp_parser.h"
#include "ddbase/network/ddtcp.h"
#include "ddbase/stream/ddstream_view.h"
#include "ddbase/network/dddns_factory.h"
#include "ddbase/network/ddtls.h"
#include "ddbase/network/ddcert.h"
#include <memory>
#include <atomic>

namespace NSP_DD {

struct ddhttp_recv_body_result
{
    bool successful = false;
    bool end = false;
    const void* buff = nullptr;
    s32 buff_size = 0;
};

template <class SNED_HEADER, class RECV_HEADER>
class ddhttp_socket
{
    DDNO_COPY_MOVE(ddhttp_socket);
protected:
    ddhttp_socket();

public:
    using ddhttp_socket_parser = ddhttp_parser<RECV_HEADER>;
    virtual ~ddhttp_socket();
    void close_socket();
    inline SOCKET get_socket()
    {
        if (m_socket == nullptr) {
            return INVALID_SOCKET;
        }
        return m_socket->get_socket();
    }

    // for client
    bool init_tls(const std::string& host);
    // for server
    bool init_tls(const std::shared_ptr<ddcert>& cert);
    ddtls* get_tls() const;

    ddcoroutine<bool> https_hand_shake(ddexpire expire = ddexpire::never);

    ddcoroutine<bool> send_head(const SNED_HEADER& head, ddexpire expire = ddexpire::never);

    // buff 在callback返回前不能被释放
    ddcoroutine<bool> send_body(void* buff, s32 buff_size, ddexpire expire = ddexpire::never);

    // head 为 nullptr 时候表示失败
    ddcoroutine<const RECV_HEADER*> recv_head(ddexpire expire = ddexpire::never);

    // 在循环中调用，直到返回ddhttp_recv_body_result::end == true或者ddhttp_recv_body_result::successful == false
    // @note: 当ddhttp_recv_body_result::successful == true时候, ddhttp_recv_body_result::buff也是可能为nullptr的
    ddcoroutine<ddhttp_recv_body_result> recv_body(ddexpire expire = ddexpire::never);

    // 当我们不关心body的时候，可以调用这个函数，如果该函数返回false，表示有错误发生
    ddcoroutine<bool> recv_to_end(ddexpire expire = ddexpire::never);

    // 用来清理缓存
    void reset();

protected:
    ddcoroutine<std::tuple<bool, s32>> send_inner(void* buff, s32 buff_size, ddexpire expire);
    ddcoroutine<std::tuple<bool, s32>> recv_inner(void* buff, s32 buff_size, ddexpire expire);
    ddasync_caller m_async_caller = nullptr;
    std::shared_ptr<ddtcp_socket> m_socket;
    ddhttp_socket_parser m_parser;

    // tls
    std::shared_ptr<ddtls> m_tls;

    // 接受的解密后的数据
    ddbuff m_recv_raw_buff;

};

using ddhttp_client_socket = ddhttp_socket<ddhttp_request_header, ddhttp_response_header>;
using ddhttp_server_socket = ddhttp_socket<ddhttp_response_header, ddhttp_request_header>;

} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_socket_h_