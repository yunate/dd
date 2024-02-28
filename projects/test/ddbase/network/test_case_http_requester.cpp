#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/dddns_factory.h"
#include "ddbase/network/http/ddhttp_client.h"
#include "ddbase/network/ddnetwork_utils.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddio.h"
#include "ddbase/thread/ddtask_thread.h"
#include "ddbase/str/ddurl.hpp"
#include "ddbase/windows/ddmoudle_utils.h"
#include "ddbase/file/ddpath.h"

#include "ddbase/ddtime.h"
#include <iostream>

namespace NSP_DD {
static void async_caller(const std::function<void()>& task)
{
    static ddtask_thread_pool g_thread_pool{ 8 };
    g_thread_pool.push_task(task);
}

static std::unique_ptr<ddfile> get_output_file()
{
    std::wstring file_path = ddpath::join(ddpath::parent(ddmoudle_utils::get_moudle_pathW()), L".\\test\\html.zip");
    ddpath::expand(file_path);
    if (!dddir::is_path_exist(ddpath::parent(file_path))) {
        if (!dddir::create_dir_ex(ddpath::parent(file_path))) {
            ddcout(ddconsole_color::red) << "create_dir_ex failure: " << ddpath::parent(file_path) << ", lasterror: " << dderror_code::get_last_error_msga() << "\n";
            return nullptr;
        }
    } else {
        dddir::delete_path(file_path);
    }
    std::unique_ptr<ddfile> file(ddfile::create_utf8_file(file_path));
    if (file == nullptr) {
        ddcout(ddconsole_color::red) << "open file failure: " << file_path << ", lasterror: " << dderror_code::get_last_error_msga() << "\n";
        return nullptr;
    }

    file->resize(0);
    return std::move(file);
}

static ddcoroutine<std::string> get_ip(dddns_factory* factory, const ddurl& url)
{
    std::string return_value;
    co_await ddmaybe_co_async_call(async_caller, [factory, &return_value, &url]() {
        auto dns_entry = factory->get_ip(url.host);
        if (dns_entry == nullptr || dns_entry->ip_v4s.empty()) {
            return_value = "";
        } else {
            return_value = dns_entry->ip_v4s[0];
        }
    });
    co_return return_value;
}

static ddcoroutine<void> co_http(ddiocp_with_dispatcher* iocp)
{
    //ddexpire expire = ddexpire::form_timeout(10000);
    ddexpire expire = ddexpire::never;
    ddexec_guard guard([iocp]() {
        iocp->notify_close();
    });

    ddtimer timer;
    auto dns = dddns_factory::create_instance();
    auto http_client = ddhttp_client::create_inst(iocp);
    if (http_client == nullptr) {
        ddcout(ddconsole_color::red) << "ddhttp_client::create_inst failure: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    ddurl url;
    parse_url("https://www.baidu.com", url);
    DDASSERT(url.is_valid());
    bool use_tls = (ddstr::lower(url.scheme.c_str()) == "https");
    ddaddr addr;
    addr.port = use_tls ? 443 : 80;
    addr.ip = co_await get_ip(dns.get(), url);

    auto http = co_await http_client->connect_to(addr, use_tls ? url.host : "", expire);
    if (http == nullptr) {
        ddcout(ddconsole_color::red) << "connect_to failure: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    auto requester = ddhttp_requester::create_inst(http, url.host);
    if (requester == nullptr) {
        ddcout(ddconsole_color::red) << "make_get_requester failure: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    ddcout(ddconsole_color::cyan) << ddstr::format("connect cost:%d \n", timer.get_time_pass() / 1000000);
    timer.reset();

    {
        const ddhttp_response_header* header = co_await requester->get(url.uri(), expire);
        if (header == nullptr) {
            ddcout(ddconsole_color::red) << "recv_head failure: " << dderror_code::get_last_error_msga() << "\n";
            co_return;
        }

        ddcout(ddconsole_color::gray) << ">>>>>>>>response header>>>>>>>>\r\n";
        ddcout(ddconsole_color::green) << header->to_str();
        ddcout(ddconsole_color::gray) << "<<<<<<<<response header:<<<<<<<<\r\n";

        auto file = get_output_file();
        if (file == nullptr) {
            co_return;
        }

        while (true) {
            auto it = co_await requester->recv_body(expire);

            if (!it.successful) {
                ddcout(ddconsole_color::red) << "recv_body failure: " << dderror_code::get_last_error_msga() << "\n";
                co_return;
            }

            if (it.buff_size > 0) {
                file->write((u8*)it.buff, it.buff_size);
            }

            if (it.end) {
                break;
            }
        }

        ddcout(ddconsole_color::green) << "successful!" << "\n";
    }
    ddcout(ddconsole_color::cyan) << ddstr::format("the first request cost:%d \n", timer.get_time_pass() / 1000000);
    timer.reset();

    for (s32 i = 0; i < 1000; ++i)
    {
        const ddhttp_response_header* header = co_await requester->get(url.uri(), expire);
        if (header == nullptr) {
            ddcout(ddconsole_color::red) << "recv_head failure: " << dderror_code::get_last_error_msga() << "\n";
            co_return;
        }

        //ddcout(ddconsole_color::gray) << ">>>>>>>>response header>>>>>>>>\r\n";
        //ddcout(ddconsole_color::green) << header->to_str();
        //ddcout(ddconsole_color::gray) << "<<<<<<<<response header:<<<<<<<<\r\n";

        auto file = get_output_file();
        if (file == nullptr) {
            co_return;
        }

        while (true) {
            auto it = co_await requester->recv_body(expire);

            if (!it.successful) {
                ddcout(ddconsole_color::red) << "recv_body failure: " << dderror_code::get_last_error_msga() << "\n";
                co_return;
            }

            if (it.buff_size > 0) {
                file->write((u8*)it.buff, it.buff_size);
            }

            if (it.end) {
                break;
            }
        }

        ddcout(ddconsole_color::green) << "successful!" << "\n";
    }
    ddcout(ddconsole_color::cyan) << ddstr::format("the second request cost:%d \n", timer.get_time_pass() / 1000000);
    co_return;
}

DDTEST(test_case_http_requester, http)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance(async_caller);
    if (iocp == nullptr) {
        return;
    }

    ddnetwork_initor initor;
    if (!initor.init()) {
        DDASSERT(false);
    }

    ddcoroutine_run(co_http(iocp.get()));

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}
} // namespace NSP_DD
