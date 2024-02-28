#ifndef ddbase_network_ddsocket_auto_reciver_async_h_
#define ddbase_network_ddsocket_auto_reciver_async_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddhttp_header.h"
#include "ddbase/network/ddhttp_data_parser.h"
#include "ddbase/network/ddsocket_async.h"

namespace NSP_DD {
///////////////////////////////////////////ddsocket_auto_reciver_async///////////////////////////////////////////
class ddsocket_auto_reciver_async : public std::enable_shared_from_this<ddsocket_auto_reciver_async>
{
public:
    ddsocket_auto_reciver_async(std::shared_ptr<ddsocket_async> socket);
    virtual ~ddsocket_auto_reciver_async();

    // @return 返回true表示继续接受数据, 一般情况应该选择返回true;
    //         返回false表示停止接受数据on_error将被调用, 当不希望on_recv被再次回调时可以选择返回false
    // @note 当返回false后, 即使重新调用start_recv也不会重新开始接受数据, 该服务从此失效
    // @note 当并不需要该服务时候可以选择直接析构, 并不强制on_recv返回false
    virtual bool on_recv(const void* ctx) = 0;
    virtual void on_error() = 0;

    // 开始自动接受数据
    virtual void start_recv();

    // 设置parser,生命周期由调用者保证
    void set_parser(ddidata_parser* parser);

protected:
    void parse_recved_data(bool successful, s32 recved);
    void recv();

protected:
    std::shared_ptr<ddsocket_async> m_socket;

private:
    bool m_service_started = false;

    // parser
    ddidata_parser* m_parser = nullptr;
    std::vector<u8> m_buff;
    s32 m_buff_remain_size = 0;
};
} // namespace NSP_DD
#endif // ddbase_network_ddsocket_auto_reciver_async_h_
