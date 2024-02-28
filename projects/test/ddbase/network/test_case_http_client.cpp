
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddnetwork.h"

#include "ddbase/ddio.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/stream/ddfile_stream.h"
#include "ddbase/str/ddstr.h"
#include "ddbase/thread/ddtask_thread.h"
#include "ddbase/thread/ddevent.h"
#include "ddbase/str/ddurl_parser.h"
#include "ddbase/stream/ddmemory_stream.h"

namespace NSP_DD {
class my_http_client;

static std::map<my_http_client*, std::shared_ptr<my_http_client>> g_clients;
std::recursive_mutex m_mutex;
static bool g_stop = false;
static ddtask_thread_pool g_thread_pool(4);
static ddtask_thread_pool g_thread_pool1(4);
static s32 count = 0;
static s32 all_count = 1000000;

class my_http_client : public ddhttp_client_async
{
public:
    my_http_client(std::shared_ptr<ddsocket_async> socket) : ddhttp_client_async(socket)
    {
    }

    virtual void on_error() override
    {
        m_socket->run_in_next_iocp_loop([this]() {
            m_socket->get_iocp()->unwatch(m_socket.get());
            if (g_clients.find(this) != g_clients.end()) {
                g_clients.erase(this);
            }
        });
    }

    virtual bool on_recv(const ddhttp_response_header& header, bool all_recved, const u8* buff, s32 buff_size) override
    {
        header;
        std::string content;
        if (buff != nullptr) {
            content.append((const char*)buff, buff_size);
        }

        if (all_recved) {
            // std::cout << content.c_str() << std::endl;
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            if (g_clients.find(this) != g_clients.end()) {
                g_clients.erase(this);
            }

            count++;
            if (count == all_count) {
                g_stop = true;
            }
        }

        return true;
    }
};


void request(my_http_client* client, const std::string& uri)
{
    ddhttp_request_header request_header;
    request_header.uri = uri;
    std::shared_ptr<std::string> content(new std::string());
    *content = request_header.to_str();
    client->get_socket()->send((void*)(*content).data(), (s32)(*content).size(), [client, content](bool successful, u64) {
        if (!successful) {
            client->on_error();
        }
    });
}

DDTEST(test_case_http_client, http_client)
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

    auto client_pool = ddtcp_connector_async::create_instance(false, *iocp);

    ddaddr addr;
    addr.ip = "127.0.0.1";
    addr.port = 8080;
    client_pool->connect(addr, [](std::shared_ptr<ddsocket_async> socket) {
        DDASSERT(socket != nullptr);
        auto client = new my_http_client(socket);
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            g_clients[client] = std::shared_ptr<my_http_client>(client);
        }
        for (s32 i = 0; i < 1000; ++i) {
            request(client, "/index.html");
        }
    });

    while (!g_stop) {
        if (!iocp->dispatch(1000)) {
            break;
        }
    }

    g_thread_pool.stop_all();
    auto service = std::move(g_clients);
}

static ddevent event1;
static s32 current_running_count = 0;

void request1(const std::string& uri, ddiocp_with_dispatcher& iocp)
{
    while (current_running_count >= 30) {
        event1.wait();
    }

    ++current_running_count;

    auto client_pool = ddtcp_connector_async::create_instance(false, iocp);

    ddaddr addr;
    addr.ip = "127.0.0.1";
    addr.port = 8080;
    client_pool->connect(addr, [uri](std::shared_ptr<ddsocket_async> socket) {
        DDASSERT(socket != nullptr);
        auto client = new my_http_client(socket);
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            g_clients[client] = std::shared_ptr<my_http_client>(client);
        }
        ddhttp_request_header request_header;
        request_header.uri = uri;
        std::shared_ptr<std::string> content(new std::string());
        client->get_socket()->send((void*)(*content).data(), (s32)(*content).size(), [client, content](bool successful, u64) {
            if (!successful) {
                client->on_error();
            }
        });
    });
}

DDTEST(test_case_http_client1, http_client)
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
    ddiocp_with_dispatcher* piocp = iocp.get();

    std::thread t([piocp](){
        while (!g_stop) {
            if (!piocp->dispatch(1000)) {
                break;
            }
        }
    });

    for (s32 i = 0; i < all_count / 10; ++i) {
        request1("/index.html", *piocp);
    }

    t.join();
    g_thread_pool.stop_all();
    auto service = std::move(g_clients);
}

bool request2(const std::string& uri)
{
    uri;
    ddaddr addr;
    addr.ip = "127.0.0.1";
    addr.port = 8080;
    auto connector = ddtcp_connector_sync::create_instance(addr.ipv4_6);
    if (connector == nullptr) {
        return false;
    }

    auto client = ddhttp_client_sync::create_instance(connector->connect(addr));
    if (client == nullptr) {
        return false;
    }

    ddhttp_request_header request_header;
    request_header.uri = uri;
    if (!client->send_header(request_header)) {
        return false;
    }

    ddmemory_stream ddmemory_stream;
    while (true) {
        const auto* it = client->recv();
        if (it == nullptr || it->parse_result.parse_state == dddata_parse_state::error) {
            // failed
            break;
        }

        if (it->parse_result.parse_state == dddata_parse_state::complete) {
            // successful
            break;
        }

        if (it->body_buff != nullptr && it->body_buff_size > 0) {
            ddmemory_stream.write(it->body_buff, it->body_buff_size);
        }
    }
    std::string content;
    content.resize((size_t)ddmemory_stream.size() +1);
    content[(size_t)ddmemory_stream.size()] = 0;
    ddmemory_stream.read((u8*)content.data(), (s32)ddmemory_stream.size());
    std::cout << content << std::endl;
    return true;
}

DDTEST(test_case_http_client2, http_client)
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
    ddtask_thread_pool thread_pool1(30);

    for (s32 i = 0; i < 30; ++i) {
        thread_pool1.push_task([](){
            for (s32 i = 0; i < all_count / 10 / 30; ++i) {
                request2("/index.html");
            }
            return true;
        });
    }
}

bool request3(const std::string& str_url)
{
    ddurl url;
    parse_url(str_url, url);
    auto dns_entry = DDLAZY_INSTANCE(dddns_factory).get_ip_sync(url.m_host);
    if (dns_entry == nullptr) {
        return false;
    }

    ddaddr addr;
    addr.ip = dns_entry->ip_v4s[0];
    addr.port = 80;
    auto connector = ddtcp_connector_sync::create_instance(addr.ipv4_6);
    if (connector == nullptr) {
        return false;
    }

    auto client = ddhttp_client_sync::create_instance(connector->connect(addr));
    if (client == nullptr) {
        return false;
    }

    ddhttp_request_header request_header;
    request_header.uri = url.m_path;
    if (request_header.uri.empty()) {
        request_header.uri = "/";
    }
    request_header.kvs["Accept"].push_back("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    request_header.kvs["Accept-Encoding"].push_back("gzip, deflate");
    request_header.kvs["Accept-Language"].push_back("zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6");
    request_header.kvs["Connection"].push_back("keep-alive");
    request_header.kvs["Host"].push_back(addr.ip);
    request_header.kvs["Upgrade-Insecure-Requests"].push_back("1");
    request_header.kvs["User-Agent"].push_back("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36");
    if (!client->send_header(request_header)) {
        return false;
    }

    ddfile* file = ddfile::create_utf8_file(L"C:\\Users\\denghuiyu\\Desktop\\test.zip");
    file->resize(0);
    std::vector<u8> buff;
    buff.resize(1024);

    while (true) {
        const auto* it = client->recv();
        if (it == nullptr || it->parse_result.parse_state == dddata_parse_state::error) {
            // failed
            break;
        }

        if (it->parse_result.parse_state == dddata_parse_state::complete) {
            // successful
            break;
        }

        if (it->body_buff != nullptr && it->body_buff_size > 0) {
            file->write(it->body_buff, it->body_buff_size);
        }
    }

    delete file;
    return true;
}

DDTEST(test_case_http_client3, http_client)
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

    request3("http://www.baidu.com");
}
} // namespace NSP_DD
