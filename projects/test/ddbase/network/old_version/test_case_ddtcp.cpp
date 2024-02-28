#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddnetwork.h"
#include "ddbase/iocp/ddiocp.h"

#include <iostream>

namespace NSP_DD {
struct client
{
    std::shared_ptr<ddsocket_async> socket = nullptr;
    std::vector<u8> send_buff;
    std::vector<u8> recv_buff;

    client(std::shared_ptr<ddsocket_async> s)
    {
        socket = s;
        recv_buff.resize(1024);
        send_buff.resize(1024);
        send_buff = { 'a', 'b', 'c' };
    }

    ~client()
    {
    }
};

DDTEST(test_case_ddtcp_accepter_async, ddiocp_socket_server)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }

    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        DDASSERT(false);
    }

    // "0.0.0.0" "0000:0000:0000:0000:0000:0000:0000:0000"
    auto server = ddtcp_accepter_async::create_instance(ddaddr{ "0.0.0.0", 8888, true}, *iocp);
    if (server == nullptr) {
        DDASSERT(false);
    }

    std::function<void(std::shared_ptr<ddsocket_async>)> callback = [&](std::shared_ptr<ddsocket_async> socket) {
        if (socket != nullptr) {
            client* c = new client(socket);
            if (!c->socket->test_socket()) {
                return;
            }
            c->socket->send(c->send_buff.data(), (s32)c->send_buff.size(), [](bool successful, u64 sended) {
                successful; sended;
            });


            c->socket->recv(c->recv_buff.data(), (s32)c->recv_buff.size(), [](bool successful, u64 readed) {
                successful; readed;
            });
            delete c;
        } else {
            // 如果 dderror_code::get_last_error() == dderror_code::time_out
            // 说明超时了
            DWORD error_code = dderror_code::get_last_error();
            if (error_code == dderror_code::time_out) {
                int i = 0;
                ++i;
                DDASSERT(true);
            }
        }

        if (!server->accept_async(callback)) {
            DDASSERT(false);
        }
    };

    if (!server->accept_async(callback)) {
        DDASSERT(false);
    }

    while (true) {
        if (!iocp->dispatch(1000)) {
            break;
        }
    }

}

DDTEST(test_case_ddtcp_connector_async, ddiocp_socket_client)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }

    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        DDASSERT(false);
    }

    auto client_pool = ddtcp_connector_async::create_instance(false, *iocp);
    client_pool->connect(ddaddr{ "::1", 8888 , false }, [](std::shared_ptr<ddsocket_async> socket) {
        DDASSERT(socket != nullptr);
        std::vector<u8>* buff = new std::vector<u8>(1024);
        socket->recv(buff->data(), (s32)buff->size(), [buff](bool successful, u64 readed) {
            successful; readed;
            delete buff;
            int i = 0;
            ++i;
        });

        std::vector<u8>* buff1 = new std::vector<u8> {'d', 'e', 'f'};
        socket->send(buff1->data(), (s32)buff1->size(), [buff1](bool successful, u64 sended) {
            successful; sended;
            delete buff1;
        });
    });

    while (true) {
        if (!iocp->dispatch(1000)) {
            break;
        }
    }
}

DDTEST(test_case_ddtcp_accepter_sync, ddiocp_socket_server)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }
    auto server = ddtcp_accepter_sync::create_instance(ddaddr {"0.0.0.0", 8888, true});
    if (server == nullptr) {
        DDASSERT(false);
    }

    while (true) {
        auto client = server->accept();
        if (client == nullptr) {
            DDASSERT(false);
        }
        while (true) {
            std::vector<u8> buff(1024);
            auto readed = client->recv(buff.data(), buff.size());
            if (readed == 0) {
                break;
            }
            buff[0] = 'b';
            buff[1] = 'b';
            buff[2] = 'b';
            auto sended = client->send(buff.data(), readed);
            if (sended == 0) {
                break;
            }
        }
    }
}

DDTEST(test_case_ddtcp_connector_sync, ddiocp_socket_client)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }
    auto pool = ddtcp_connector_sync::create_instance(true);
    auto client = pool->connect(ddaddr{ "127.0.0.1", 8888, true });
    if (client == nullptr) {
        DDASSERT(false);
    }

    std::vector<u8> buff(1024);
    buff[0] = 'a';
    buff[1] = 'a';
    buff[2] = 'a';
    auto sended = client->send(buff.data(), buff.size());
    if (sended == 0) {
        DDASSERT(false);
    }

    if (!client->recv(buff.data(), buff.size())) {
        DDASSERT(false);
    }
}
} // namespace NSP_DD
