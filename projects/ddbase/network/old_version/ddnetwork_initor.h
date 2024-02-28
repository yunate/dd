#ifndef ddbase_network_ddnetwork_initor_h_
#define ddbase_network_ddnetwork_initor_h_
#include "ddbase/dddef.h"
#include "ddbase/thread/ddasync.h"
#include "ddbase/ddnocopyable.hpp"
namespace NSP_DD {

class ddnetwork_initor
{
    DDNO_COPY_MOVE(ddnetwork_initor)
public:
    ddnetwork_initor() = default;
    ~ddnetwork_initor();

    // 初始化网络环境:
    //  WSAStartup
    //  dddns_factory 初始化
    // @param[in] v0, v1 版本
    // @param[in] async_caller 设置线程池, 如果不需要线程池可以为nullptr
    //      e.g.:
    //      init([](const std::function<void()>& task) {
    //          g_thread_pool.push_task([task]() {
    //              task();
    //              return true;
    //          });
    //      })
    // @return 是否成功, 返回false时候可以通过init_result函数获得具体原件
    // @note v0, v1, init_result 参见 ::WSAStartup 函数说明
    bool init(const ddasync_caller& async_caller, u8 v0 = 2, u8 v1 = 2);

private:
    bool init_wsa(u8 v0 = 2, u8 v1 = 2);
    int m_WSAStartup_result = -1;
};

} // namespace NSP_DD
#endif // ddbase_network_ddnetwork_initor_h_
