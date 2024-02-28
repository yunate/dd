#ifndef ddbase_network_ddudp_h_
#define ddbase_network_ddudp_h_
#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/iocp/ddiocp_io_dispatch.h"
#include "ddbase/network/ddnetwork_utils.h"
#include <memory>
#include <map>

namespace NSP_DD {
class ddudp_socket : public ddiocp_io_dispatch_base
{
    DDNO_COPY_MOVE(ddudp_socket);
    ddudp_socket() = default;
public:
    static std::shared_ptr<ddudp_socket> create_inst(ddiocp_with_dispatcher* iocp, bool ipv4_6 = true);
    virtual ~ddudp_socket();
    void close_socket();
    inline SOCKET get_socket()
    {
        return m_socket;
    }

    ddcoroutine<std::tuple<bool, s32>> send_to(void* buff, s32 buff_size, const ddaddr& addr, ddexpire expire = ddexpire::never);

    // 调用recv_from前需要使用listen函数监听某个端口
    bool listen(u16 port);
    ddcoroutine<std::tuple<bool, s32>> recv_from(void* buff, s32 buff_size, ddaddr& addr, ddexpire expire = ddexpire::never);

// from ddiocp_timeout_dispatch
    HANDLE get_handle() override
    {
        return (HANDLE)m_socket;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    bool m_ipv4_6 = true;

    // send context
    WSABUF m_send_wsa_buff{ 0, nullptr };

    // recv context
    WSABUF recv_wsa_buff{ 0, nullptr };
    char m_addr_buff[sizeof(sockaddr_in) > sizeof(sockaddr_in6) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6)] = { 0 };
    int m_addr_buff_len = sizeof(m_addr_buff);
    DWORD m_flags = 0;

#ifdef _DEBUG
    bool m_has_listened = false;
#endif
};

} // namespace NSP_DD
#endif // ddbase_network_ddudp_h_