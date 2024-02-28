#ifndef ddbase_network_http_ddhttp_server_h_
#define ddbase_network_http_ddhttp_server_h_
#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include "ddbase/network/http/ddhttp_parser.h"
#include "ddbase/network/ddtcp.h"
#include "ddbase/stream/ddstream_view.h"
#include "ddbase/network/dddns_factory.h"
#include "ddbase/network/ddtls.h"
#include <memory>
//////////////////////////////////////////////////////////ddhttp_server//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
//class ddhttp_server_client
//{
//public:
//    // head 为 nullptr 时候表示失败
//    void recv_head(const std::function<void(const ddhttp_response_header* head)>& callback, ddexpire expire = ddexpire::never);
//    ddcoroutine<const ddhttp_response_header*> recv_head(ddexpire expire = ddexpire::never);
//
//    struct recv_body_result
//    {
//        bool successful = false;
//        bool end = false;
//        const void* buff = nullptr;
//        s32 buff_size = 0;
//    };
//    void recv_body(const std::function<void(const recv_body_result&)>& callback, ddexpire expire = ddexpire::never);
//    ddcoroutine<recv_body_result> recv_body(ddexpire expire = ddexpire::never);
//
//    void https_hand_shake(const std::string& host, const std::function<void(bool successful)>& callback, ddexpire expire = ddexpire::never);
//    ddcoroutine<bool> https_hand_shake(const std::string& host, ddexpire expire = ddexpire::never);
//
//    void send_head(const ddhttp_request_header& head, const std::function<void(bool successful)>& callback, ddexpire expire = ddexpire::never);
//    ddcoroutine<bool> send_head(const ddhttp_request_header& head, ddexpire expire = ddexpire::never);
//
//    // buff 在callback返回前不能被释放
//    void send_body(void* buff, s32 buff_size, const std::function<void(bool successful)>& callback, ddexpire expire = ddexpire::never);
//    ddcoroutine<bool> send_body(void* buff, s32 buff_size, ddexpire expire = ddexpire::never);
//
//    // stream 在callback返回false或者all_sended为true前不能被释放
//    void send_stream(ddistream_view* stream, const std::function<void(bool successful, s32 byte, bool all_sended)>& callback, ddexpire expire = ddexpire::never);
//
//private:
//    std::shared_ptr<ddtcp_socket> m_socket;
//    std::unique_ptr<ddhttp_request_parser> m_parser;
//};
} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_server_h_