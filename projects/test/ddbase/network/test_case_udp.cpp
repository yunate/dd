#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddudp.h"
#include "ddbase/network/ddnetwork_utils.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddio.h"

#include "ddbase/ddtime.h"
#include <iostream>

namespace NSP_DD {
ddcoroutine<void> co_udp_client(ddiocp_with_dispatcher* iocp)
{
    auto socket = ddudp_socket::create_inst(iocp);
    if (socket == nullptr) {
        ddcout(ddconsole_color::red) << "ddudp_socket::create_inst failure: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    ddaddr addr;
    addr.ip = "127.0.0.1";
    addr.port = 8080;

    while (true) {
        ddcout(ddconsole_color::gray) << "please enter message, enter close to close the socket.\n";
        std::string write;
        ddcin() >> write;

        ddpickle close_pkg;
        close_pkg << write;
        auto tuple = co_await socket->send_to((u8*)(close_pkg.get_buff().data()), (s32)close_pkg.get_buff().size(), addr, ddexpire::form_timeout(1000));

        if (!std::get<0>(tuple)) {
            ddcout(ddconsole_color::red) << "send_to failure: " << dderror_code::get_last_error_msga() << "\n";
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

DDTEST(test_case_udp_client, client)
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

    ddcoroutine_run(co_udp_client(iocp.get()));

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

ddcoroutine<void> co_udp_server(ddiocp_with_dispatcher* iocp)
{
    auto socket = ddudp_socket::create_inst(iocp);
    if (socket == nullptr) {
        ddcout(ddconsole_color::red) << "ddudp_socket::create_inst failure: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    if (!socket->listen(8080)) {
        ddcout(ddconsole_color::red) << "ddudp_socket::listen failure: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    while (true) {
        ddaddr addr;
        std::vector<char> buff(1024);
        auto tuple = co_await socket->recv_from((u8*)(buff.data()), (s32)buff.size(), addr, ddexpire::form_timeout(1000));
        if (!std::get<0>(tuple) ||
            ddpickle::get_next_pickle_size((u8*)buff.data(), std::get<1>(tuple)) == 0) {
            if (dderror_code::get_last_error() == dderror_code::time_out) {
                continue;
            }
            ddcout(ddconsole_color::red) << "recv_from failure: " << dderror_code::get_last_error_msga() << "\n";
            break;
        }

        ddpickle pkg((u8*)buff.data(), std::get<1>(tuple));
        std::string readed;
        pkg >> readed;
        ddcout(ddconsole_color::purple) << addr.ip << ": " << readed << L"\n";
        if (readed == "close-all") {
            socket->get_iocp()->notify_close();
            break;
        }
    }
}

DDTEST(test_case_udp_server, server)
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

    ddcoroutine_run(co_udp_server(iocp.get()));

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}
} // namespace NSP_DD
