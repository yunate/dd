#include "ddbase/stdafx.h"
#include "ddbase/network/ddhttp_auto_reciver_async.h"

#include <memory>
namespace NSP_DD {
///////////////////////////////////////////ddhttp_auto_reciver_async///////////////////////////////////////////
template <class HEADER>
ddhttp_auto_reciver_async<HEADER>::ddhttp_auto_reciver_async(std::shared_ptr<ddsocket_async> socket) :
    ddsocket_auto_reciver_async(socket)
{
    ddsocket_auto_reciver_async::set_parser(&m_parser);
}

template<class HEADER>
ddsocket_async* ddhttp_auto_reciver_async<HEADER>::get_socket()
{
    return m_socket.get();
}

template <class HEADER>
bool ddhttp_auto_reciver_async<HEADER>::on_recv(const void* ctx)
{
    DDASSERT(ctx != nullptr);
    const PARSER::context* parser_ctx = (const PARSER::context*)ctx;
    if (!parser_ctx->head_parsed) {
        return true;
    }

    return on_recv(parser_ctx->header, parser_ctx->parse_result.parse_state == dddata_parse_state::complete, parser_ctx->body_buff, parser_ctx->body_buff_size);
}

template class ddhttp_auto_reciver_async<ddhttp_request_header>;
template class ddhttp_auto_reciver_async<ddhttp_response_header>;
} // namespace NSP_DD
