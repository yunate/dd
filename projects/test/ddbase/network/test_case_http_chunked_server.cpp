
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddnetwork.h"

#include "ddbase/ddio.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/stream/ddfile_stream.h"
#include "ddbase/stream/ddmemory_stream.h"
#include "ddbase/str/ddstr.h"
#include "ddbase/thread/ddtask_thread.h"


namespace NSP_DD {
class chunked_http_service;

static std::map<chunked_http_service*, std::shared_ptr<chunked_http_service>> g_services;
static bool g_stop = false;
static ddtask_thread_pool g_thread_pool(8);
int cc = 0;
class chunked_http_service : public ddhttp_service
{
public:
    chunked_http_service(std::shared_ptr<ddsocket_async> socket) :
        ddhttp_service(socket)
    {
        m_base_path = ddpath::join(LR"(F:\My\test_folder\test_http_chunked\)", ddstr::format(L"%d", cc));
        ++cc;
        if (dddir::is_path_exist(m_base_path)) {
            dddir::delete_path(m_base_path);
        }
        m_file_stream = ddfile_stream::create_instance(m_base_path);
    }

    ~chunked_http_service()
    {
        m_file_stream.reset();
        dddir::delete_path(m_base_path);
    }

    virtual bool on_recv(const ddhttp_request_header& header, bool all_recved, const u8* buff, s32 buff_size) override
    {
        if (buff != nullptr && buff_size != 0) {
            m_file_stream->write(buff, buff_size);
        }

        if (!all_recved) {
            return true;
        }

        if (header.uri == "/close") {
            g_stop = true;
            return false;
        }

        // make data
        std::shared_ptr<std::string> response_str = std::make_shared<std::string>();
        ddhttp_response_header response_header;
        response_header.version = "HTTP/1.1";
        response_header.state = 200;
        response_header.state_str = "OK";
        ddtime now = ddtime::now_fmt();
        response_header.kvs["Date"].push_back(ddtime::fmt_gmt(now));
        response_header.kvs["Server"].push_back("ddserver");
        response_header.kvs["Content-Type"].push_back("text/html; charset=utf-8");
        response_header.kvs["Content-Encoding"].push_back("gzip");
        response_header.kvs["Connection"].push_back("keep-alive");
        response_header.kvs.set_transfer_encoding("chunked");

        // send the header
        *response_str = response_header.to_str();
        m_socket->send((void*)(*response_str).data(), (s32)(*response_str).size(), [this, response_str](bool successful, u64) {
            if (!successful) {
                on_error();
            }
        });

        // send chunked data
        s32 size = (s32)m_file_stream->size();
        m_file_stream->seek(0, SEEK_SET);
        u8 file_buff[1024];

        while (size > 0) {
            std::shared_ptr<ddmemory_stream> chunked = std::make_shared<ddmemory_stream>();
            s32 readed = m_file_stream->read(file_buff, 1024);
            std::string chunken_len_str = ddstr::format("%x\r\n", readed);
            chunked->write((u8*)chunken_len_str.data(), (s32)chunken_len_str.size());
            chunked->write(file_buff, readed);
            chunked->write((u8*)"\r\n", 2);
            chunked->seek(0, SEEK_SET);
            m_socket->send_stream(chunked.get(), [this, chunked](bool successful, bool all_sended, u64) {
                all_sended;
                if (!successful) {
                    on_error();
                }
                return true;
            });
            size -= readed;
            //std::cout << readed << " ";
        }

        std::cout << std::endl;
        std::shared_ptr<std::string> chunk_end = std::make_shared<std::string>();
        *chunk_end = "0\r\n\r\n";
        m_socket->send((void*)(*chunk_end).data(), (s32)(*chunk_end).size(), [this, chunk_end](bool successful, u64) {
            if (!successful) {
                on_error();
            }
        });
        return true;
    }

    virtual void on_error() override
    {
        m_socket->run_in_next_iocp_loop([this]() {
            m_socket->get_iocp()->unwatch(m_socket.get());
            if (g_services.find(this) != g_services.end()) {
                g_services.erase(this);
            }
        });
    }

private:
    std::wstring m_base_path;
    std::shared_ptr<ddfile_stream> m_file_stream;
};

DDTEST(test_case_http_chunked_server, tcp_server)
{
    auto async_caller = [](const std::function<void()>& task) {
        g_thread_pool.push_task([task]() {
            task();
            return true;
        });
    };
    ddnetwork_initor initor;
    if (!initor.init(async_caller)) {
        DDASSERT(false);
    }

    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        DDASSERT(false);
    }
    auto server = ddtcp_server::create_instance(*iocp, [](std::shared_ptr<ddsocket_async> socket) {
        if (socket == nullptr) {
            return;
        }

        auto service = new chunked_http_service(socket);
        g_services[service] = std::shared_ptr<chunked_http_service>(service);
        service->start_recv();
    });

    server->listen_port(8080, true, []() {
        std::cout << "listen port 8080 failed" << std::endl;
    });

    server->listen_port(443, true, []() {
        std::cout << "listen port 443 failed" << std::endl;
    });

    while (!g_stop) {
        if (!iocp->dispatch(1000)) {
            break;
        }
    }

    g_thread_pool.stop_all();
    auto service = std::move(g_services);
}

} // namespace NSP_DD
