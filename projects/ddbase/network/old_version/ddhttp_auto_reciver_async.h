#ifndef ddbase_network_ddhttp_auto_reciver_async_h_
#define ddbase_network_ddhttp_auto_reciver_async_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddsocket_auto_reciver_async.h"

namespace NSP_DD {
///////////////////////////////////////////ddhttp_auto_reciver_async///////////////////////////////////////////
template <class HEADER>
class ddhttp_auto_reciver_async : public ddsocket_auto_reciver_async
{
public:
    using PARSER = typename ddhttp_data_parser<HEADER>;
    ddhttp_auto_reciver_async(std::shared_ptr<ddsocket_async> socket);
    virtual ~ddhttp_auto_reciver_async() = default;

    // @param[in] header 请求头
    // @param[in] all_recved, 是否完全接受完毕
    // @param[in] body_slice_buff, 本次接受到的body部分,可能为nullptr, 内存内部维护
    // @param[in] body_slice_buff_size, body_slice_buff 的大小
    // @return 返回true表示继续接受数据, 一般情况应该选择返回true;
    //         返回false表示停止接受数据on_error将被调用, 当不希望on_recv被再次回调时可以选择返回false
    // @note 当返回false后, 即使重新调用start_service也不会重新开始接受数据, 该服务从此失效
    // @note 当并不需要该服务时候可以选择直接析构, 并不强制on_recv返回false
    virtual bool on_recv(const HEADER& header, bool all_recved, const u8* body_slice_buff, s32 body_slice_buff_size) = 0;
    virtual void on_error() = 0;
    ddsocket_async* get_socket();

protected:
    bool on_recv(const void* ctx) override;

    // parse
    PARSER m_parser;
};

using ddhttp_service = ddhttp_auto_reciver_async<ddhttp_request_header>;
using ddhttp_client_async = ddhttp_auto_reciver_async<ddhttp_response_header>;
} // namespace NSP_DD

#endif // ddbase_network_ddhttp_auto_reciver_async_h_
