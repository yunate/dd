
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddnetwork.h"
#include "ddbase/ddio.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/stream/ddfile_stream.h"
#include "ddbase/str/ddstr.h"

namespace NSP_DD {

class tcp_file_service
{
public:
    tcp_file_service(std::shared_ptr<ddsocket_async> socket)
    {
        m_buff.resize(2048);
        m_buff_remain_size = 0;
        m_socket = socket;
    }

    void recv()
    {
        m_socket->recv(m_buff.data() + m_buff_remain_size, (s32)(m_buff.size() - m_buff_remain_size), [this](bool successful, u64 recved) {
            on_recv(successful, (s32)(recved));
        });
    }

    void on_error()
    {
        delete this;
    }

    void on_recv(bool successful, s32 recved)
    {
        if (!successful) {
            on_error();
            return;
        }

        u8* buff = m_buff.data();
        m_buff_remain_size += (s32)(recved);
        u32 pickle_size = ddpickle::get_next_pickle_size(buff, m_buff_remain_size);
        while (pickle_size != 0) {
            ddpickle pkg(buff, m_buff_remain_size);
            on_package(pkg);
            buff += pickle_size;
            m_buff_remain_size -= pickle_size;
            pickle_size = ddpickle::get_next_pickle_size(buff, m_buff_remain_size);
        }

        if (m_buff_remain_size != 0 && buff != m_buff.data()) {
            (void)::memmove(m_buff.data(), buff, m_buff_remain_size);
        }

        recv();
    }

    void on_package(ddpickle& pkg)
    {
        std::string path;
        if (!(pkg >> path) || path.empty()) {
            on_error();
            return;
        }

        auto stream = ddfile_stream::create_instance(ddstr::utf8_16(path));
        ddfile_stream* stream_ptr = stream.release();
        m_socket->send_stream(stream_ptr, [stream_ptr, this](bool successful, bool all_sended, u64){
            if (all_sended) {
                delete stream_ptr;
            }
            std::cout << "__" << successful << "__" << dderror_code::get_last_error_msga << std::endl;
            on_error();
            return true;
        });
    }

private:
    std::vector<u8> m_buff;
    s32 m_buff_remain_size = 0;
    std::shared_ptr<ddsocket_async> m_socket;
};

DDTEST(test_case_ddtcp_server, tcp_server)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }

    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        DDASSERT(false);
    }

    auto server = ddtcp_server::create_instance(*iocp, [](std::shared_ptr<ddsocket_async> socket){
        if (socket == nullptr) {
            return;
        }

        auto service = new tcp_file_service(socket);
        service->recv();
    });

    server->listen_port(8080, true, []() {
        std::cout << "listen port 8080 failed" << std::endl;
    });

    while (true) {
        if (!iocp->dispatch(1000)) {
            break;
        }
    }
}

} // namespace NSP_DD



