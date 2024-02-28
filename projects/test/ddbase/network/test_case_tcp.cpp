#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddtcp.h"
#include "ddbase/network/ddnetwork_utils.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddio.h"

#include "ddbase/ddtime.h"
#include <iostream>

namespace NSP_DD {
ddcoroutine<void> co_tcp_client(ddiocp_with_dispatcher* iocp)
{
    auto connector = ddtcp_connector::create_inst(iocp);
    if (connector == nullptr) {
        co_return;
    }

    ddaddr addr;
    addr.ip = "127.0.0.1";
    addr.port = 8080;
    auto socket = co_await connector->connect_to(addr, ddexpire::form_timeout(1000));
    if (socket == nullptr) {
        ddcout(ddconsole_color::red) << "connect failure: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    ddcout(ddconsole_color::green) << "connect successful!\n";
    while (true) {
        ddcout(ddconsole_color::gray) << "please enter message, enter close to close the socket.\n";
        std::string write;
        ddcin() >> write;

        ddpickle pkg;
        pkg << write;
        auto tuple = co_await socket->write((u8*)(pkg.get_buff().data()), (s32)pkg.get_buff().size(), ddexpire::form_timeout(1000));

        if (!std::get<0>(tuple)) {
            ddcout(ddconsole_color::red) << "write failure: " << dderror_code::get_last_error_msga() << "\n";
            if (dderror_code::get_last_error() != dderror_code::time_out) {
                break;
            }
        }

        if (write == "close" || write == "close-all") {
            break;
        }
    }
    iocp->notify_close();
    co_return;
}

DDTEST(test_case_tcp_client, client)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    ddnetwork_initor initor;
    if (!initor.init()) {
        DDASSERT(false);
    }

    ddcoroutine_run(co_tcp_client(iocp.get()));

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

ddcoroutine<void> co_tcp_server(ddiocp_with_dispatcher* iocp)
{
    ddaddr listen_addr;
    listen_addr.ip = ddnetwork_utils::get_zero_ip(true);
    listen_addr.port = 8080;
    auto acceptor = ddtcp_acceptor::create_inst(iocp, listen_addr);
    if (acceptor == nullptr) {
        co_return;
    }

    while (true) {
        auto socket = co_await acceptor->accept();
        if (socket == nullptr) {
            ddcout(ddconsole_color::red) << "acceptor failure: " << dderror_code::get_last_error_msga() << "\n";
            continue;
        } else {
            ddcout(ddconsole_color::green) << "acceptor successful!\n";
        }
        ddcoroutine_run([](std::shared_ptr<ddtcp_socket> socket) -> ddcoroutine<void> {
            std::vector<char> buff(10240);
            s32 buff_remain_size = 0;
            while (true) {
                auto tuple = co_await socket->read((u8*)(buff.data() + buff_remain_size), (s32)buff.size() - buff_remain_size, ddexpire::form_timeout(1000));
                if (!std::get<0>(tuple)) {
                    if (dderror_code::get_last_error() == dderror_code::time_out) {
                        continue;
                    }
                    ddcout(ddconsole_color::red) << "read failure: " << dderror_code::get_last_error_msga() << "\n";
                    break;
                }

                bool end = false;
                buff_remain_size += std::get<1>(tuple);
                char* buff_begin = buff.data();
                u32 pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
                while (pickle_size != 0) {
                    ddpickle pkg((u8*)buff_begin, pickle_size);
                    std::string readed;
                    pkg >> readed;
                    ddcout(ddconsole_color::purple) << readed << L"\n";

                    if (readed == "close") {
                        end = true;
                        break;
                    }

                    if (readed == "close-all") {
                        end = true;
                        socket->get_iocp()->notify_close();
                        break;
                    }

                    buff_begin += pickle_size;
                    buff_remain_size -= pickle_size;
                    pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
                }

                if (end) {
                    break;
                }

                if (buff_remain_size != 0 && buff_begin != buff.data()) {
                    (void)::memmove(buff.data(), buff_begin, buff_remain_size);
                }
            }
            co_return;
        }(socket));
    }
}

DDTEST(test_case_tcp_server, server)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    ddnetwork_initor initor;
    if (!initor.init()) {
        DDASSERT(false);
    }

    ddcoroutine_run(co_tcp_server(iocp.get()));

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}
} // namespace NSP_DD
