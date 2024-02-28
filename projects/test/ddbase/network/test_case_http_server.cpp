#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/dddns_factory.h"
#include "ddbase/network/http/ddhttp_server.h"
#include "ddbase/network/http/ddhttp_utils.h"
#include "ddbase/network/ddnetwork_utils.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddio.h"
#include "ddbase/thread/ddtask_thread.h"
#include "ddbase/str/ddurl.hpp"
#include "ddbase/windows/ddmoudle_utils.h"
#include "ddbase/file/ddpath.h"
#include "ddbase/file/ddfile.h"
#include "ddbase/stream/ddmemory_stream.h"

#include "ddbase/ddtime.h"
#include <iostream>
#include <mutex>

namespace NSP_DD {
static void async_caller(const std::function<void()>& task)
{
    static ddtask_thread_pool g_thread_pool{ 8 };
    g_thread_pool.push_task(task);
}

static bool g_stop = false;
static std::shared_ptr<ddhttp_server> g_server;
static std::wstring g_root_path;
static std::mutex g_mutex;

ddcoroutine<void> co_on_client_connect(std::shared_ptr<ddhttp_server_socket> item)
{
    while (true) {
        auto* head = co_await item->recv_head(ddexpire::form_timeout(1000));
        if (head == nullptr) {
            ddcout(ddconsole_color::red) << "recv_head failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
            break;
        }

        if (!co_await item->recv_to_end(ddexpire::form_timeout(1000))) {
            ddcout(ddconsole_color::red) << "recv_to_end failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
            break;
        }

        if (head->uri == "/close") {
            g_mutex.lock();
            g_stop = true;
            g_server->close_socket();
            g_mutex.unlock();
            break;
        }

        ddhttp_response_header response_header;
        response_header.version = "HTTP/1.1";
        response_header.state = 200;
        response_header.state_str = "OK";
        response_header.kvs.reset("Date", ddtime::fmt_gmt(ddtime::now_fmt()));
        response_header.kvs.reset("Server", "ddserver");
        response_header.kvs.reset("Content-Type", ddhttp_utils::get_content_type(ddstr::lower(head->uri.c_str())));

        if (head->uri == "/hello") {
            ddaddr addr;
            ddnetwork_utils::get_remote_addr(item->get_socket(), addr);
            // ddcout(ddconsole_color::green) << ddstr::format("%s:%d send hello\r\n", addr.ip.c_str(), addr.port);
            std::string txt = "hello";
            response_header.kvs.reset("Content-Length", ddstr::format("%d", txt.size()));
            if (!co_await item->send_head(response_header, ddexpire::form_timeout(1000))) {
                ddcout(ddconsole_color::red) << "send_head failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                co_return;
            }

            if (!co_await item->send_body(txt.data(), (s32)txt.size(), ddexpire::form_timeout(1000))) {
                ddcout(ddconsole_color::red) << "send_body failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                co_return;
            }
        } else {
            std::string uri = head->uri;
            if (uri == "/" || uri == "/chunked") {
                uri = "index.html";
            }
            std::unique_ptr<ddfile> file;
            std::wstring full_path = ddpath::join(g_root_path, ddstr::utf8_16(uri));
            if (dddir::is_path_exist(full_path)) {
                file.reset(ddfile::create_utf8_file(full_path));
            }
            if (file == nullptr) {
                ddcout(ddconsole_color::red) << ddstr::format(L"read file: %s failure, %s!", full_path.c_str(), dderror_code::get_last_error_msgw().c_str());
                response_header.kvs.reset("Content-Length", "0");
                response_header.state = 404;
                response_header.state_str = "Not Found";
                if (!co_await item->send_head(response_header, ddexpire::form_timeout(1000))) {
                    ddcout(ddconsole_color::red) << "send_head failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                    co_return;
                }
                continue;
            }

            s64 size = file->file_size();
            bool use_chunked = true;
            if (use_chunked) {
                /// response_header.kvs.reset("Content-Encoding", "gzip");
                response_header.kvs.reset("Connection", "keep-alive");
                response_header.kvs.set_transfer_encoding("chunked");
                if (!co_await item->send_head(response_header, ddexpire::form_timeout(1000))) {
                    ddcout(ddconsole_color::red) << "send_head failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                    co_return;
                }

                // ddcout(ddconsole_color::green) << ddstr::format("%s: chunked\r\n", head->uri.c_str());
                while (size > 0) {
                    u8 file_buff[1024];
                    s32 readed = file->read(file_buff, 1024);
                    std::string chunken_len_str = ddstr::format("%x\r\n", readed);
                    std::vector<u8> chunked((s32)chunken_len_str.size() + readed + 2);
                    ::memcpy(chunked.data(), (u8*)chunken_len_str.data(), (s32)chunken_len_str.size());
                    ::memcpy(chunked.data() + (s32)chunken_len_str.size(), file_buff, readed);
                    chunked[chunked.size() - 2] = '\r';
                    chunked[chunked.size() - 1] = '\n';
                    if (!co_await item->send_body(chunked.data(), (s32)chunked.size(), ddexpire::form_timeout(1000))) {
                        ddcout(ddconsole_color::red) << "send_body failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                        co_return;
                    }
                    size -= readed;
                }

                std::string chunk_end;
                chunk_end = "0\r\n\r\n";
                if (!co_await item->send_body(chunk_end.data(), (s32)chunk_end.size(), ddexpire::form_timeout(1000))) {
                    ddcout(ddconsole_color::red) << "send_body failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                    co_return;
                }
            } else {
                response_header.kvs.reset("Content-Length", ddstr::format("%d", size));
                if (!co_await item->send_head(response_header, ddexpire::form_timeout(1000))) {
                    ddcout(ddconsole_color::red) << "send_head failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                    co_return;
                }

                // ddcout(ddconsole_color::green) << ddstr::format("%s: not chunked\r\n", head->uri.c_str());
                while (true) {
                    u8 buff[1024];
                    s32 readed = file->read(buff, 1024);
                    if (readed <= 0) {
                        break;
                    }

                    if (!co_await item->send_body(buff, readed, ddexpire::form_timeout(1000))) {
                        ddcout(ddconsole_color::red) << "send_body failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
                        co_return;
                    }
                }
            }

        }

        if (!head->kvs.keep_alive()) {
            break;
        }
    }

    co_return;
}

static bool init_root_path()
{
    g_root_path = ddpath::join(ddpath::parent(ddmoudle_utils::get_moudle_pathW()), LR"(..\test_folder\webpage\index)");
    ddpath::expand(g_root_path);
    if (!dddir::is_path_exist(g_root_path)) {
        (void)dddir::create_dir_ex(g_root_path);
    }
    return dddir::is_path_exist(g_root_path);
}

std::shared_ptr<ddcert> create_cert()
{
    std::wstring pfx_path = ddpath::join(ddpath::parent(ddmoudle_utils::get_moudle_pathW()), LR"(..\..\projects\test\res\cret\ddm.pfx)");
    ddpath::expand(pfx_path);
    return ddcert::create_instance(pfx_path, L"123456");
}

ddcoroutine<void> co_http_server(ddiocp_with_dispatcher* iocp)
{
    if (!init_root_path()) {
        ddcout(ddconsole_color::red) << "init_root_path, lasterror: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    auto cert = create_cert();
    ddaddr addr;
    addr.ip = "0.0.0.0";
    addr.port = 8080;

    bool use_tls = false;
    g_server = ddhttp_server::create_inst(iocp, addr, use_tls ? cert : nullptr);
    if (g_server == nullptr) {
        ddcout(ddconsole_color::red) << "create_inst failure, lasterror: " << dderror_code::get_last_error_msga() << "\n";
        co_return;
    }

    while (!g_stop) {
        auto item = co_await g_server->accept(ddexpire::form_timeout(1000));
        if (item == nullptr) {
            continue;
        }

        ddcoroutine_run(co_on_client_connect(item));
    }

    g_server.reset();
    co_return;
}

DDTEST(test_case_http_server, http)
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

    ddcoroutine_run(co_http_server(iocp.get()));

    while (!g_stop) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}
} // namespace NSP_DD
