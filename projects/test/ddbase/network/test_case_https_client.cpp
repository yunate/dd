
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddnetwork.h"
#include "ddbase/str/ddurl_parser.h"
#include "ddbase/str/ddstr.h"

#include <winsock2.h>


namespace NSP_DD {
ddtimer timer;

bool request_https(const std::string& str_url)
{
    ddurl url;
    parse_url(str_url, url);
    auto dns_entry = DDLAZY_INSTANCE(dddns_factory).get_ip_sync(url.m_host);
    if (dns_entry == nullptr) {
        return false;
    }

    ddaddr addr;
    addr.ip = dns_entry->ip_v4s[0];
    if (url.m_scheme == "https") {
        addr.port = 443;
    } else if(url.m_scheme == "http") {
        addr.port = 80;
    }
    auto connector = ddtcp_connector_sync::create_instance(addr.ipv4_6);
    if (connector == nullptr) {
        return false;
    }

    std::unique_ptr<ddsocket_sync> sync_socket = connector->connect(addr);
    std::unique_ptr<ddtls_client> tls_client(new ddtls_client(ddstr::utf8_16(url.m_host)));
    std::shared_ptr<ddtls_socket_sync> tls_socket(new ddtls_socket_sync(std::move(tls_client), std::move(sync_socket)));
    if (!tls_socket->hand_shake()) {
        return false;
    }
    auto client = ddhttp_client_sync::create_instance(tls_socket);
    //auto client = ddhttp_client_sync::create_instance(std::shared_ptr<ddsocket_sync>(sync_socket.release()));
    if (client == nullptr) {
        return false;
    }
    ddhttp_request_header request_header;
    request_header.uri = url.m_path;
    if (request_header.uri.empty()) {
        request_header.uri = "/";
    }
    request_header.kvs["Accept"].push_back("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    request_header.kvs["Accept-Encoding"].push_back("gzip, deflate, br");
    request_header.kvs["Accept-Language"].push_back("zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6");
    request_header.kvs["Cache-Control"].push_back("max-age=0");
    request_header.kvs["Connection"].push_back("keep-alive");
    request_header.kvs["Host"].push_back(addr.ip);
    request_header.kvs["Sec-Ch-Ua"].push_back(R"__("Chromium";v="118", "Microsoft Edge";v="118", "Not=A?Brand";v="99")__");
    request_header.kvs["Sec-Ch-Ua-Mobile"].push_back(R"__(?0)__");
    request_header.kvs["Sec-Ch-Ua-Platform"].push_back(R"__(Windows)__");
    request_header.kvs["Sec-Fetch-Dest"].push_back(R"__(document)__");
    request_header.kvs["Sec-Fetch-Mode"].push_back(R"__(navigate)__");
    request_header.kvs["Sec-Fetch-Site"].push_back(R"__(none)__");
    request_header.kvs["Sec-Fetch-User"].push_back(R"__(?1)__");
    request_header.kvs["Upgrade-Insecure-Requests"].push_back("1");
    request_header.kvs["User-Agent"].push_back("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36 Edg/118.0.2088.57");
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

        if (it->body_buff != nullptr && it->body_buff_size > 0) {
            file->write(it->body_buff, it->body_buff_size);
        }

        if (it->parse_result.parse_state == dddata_parse_state::complete) {
            // successful
            break;
        }

    }

    delete file;
    return true;
}

DDTEST(test_case_http_clients, http_client)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }
    for (s32 i = 0; i < 10; ++i) {
        std::cout << "------------------------" << std::endl;
        timer.reset();
        request_https("https://www.baidu.com");
        std::cout << timer.get_time_pass() / 1000000 << std::endl;
    }
}

static bool g_stop = false;

class my_http_client1;
static std::recursive_mutex m_mutex;
static std::map<my_http_client1*, std::shared_ptr<my_http_client1>> g_clients;
class my_http_client1 : public ddhttp_client_async
{
public:
    my_http_client1(std::shared_ptr<ddsocket_async> socket) : ddhttp_client_async(socket)
    {
        m_file = ddfile::create_utf8_file(L"C:\\Users\\denghuiyu\\Desktop\\test.zip");
    }

    ~my_http_client1()
    {
        delete m_file;
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
        if (buff != nullptr) {
            m_file->write(buff, buff_size);
        }

        if (all_recved) {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            if (g_clients.find(this) != g_clients.end()) {
                g_clients.erase(this);
            }
            g_stop = true;
        }

        return true;
    }

    ddfile* m_file = nullptr;
};

bool request_https_async(ddtcp_connector_async* client_pool, const std::string& str_url)
{

    ddurl url;
    parse_url(str_url, url);
    auto dns_entry = DDLAZY_INSTANCE(dddns_factory).get_ip_sync(url.m_host);
    if (dns_entry == nullptr) {
        return false;
    }

    ddaddr addr;
    addr.ip = dns_entry->ip_v4s[0];
    if (url.m_scheme == "https") {
        addr.port = 443;
    } else if (url.m_scheme == "http") {
        addr.port = 80;
    }

    client_pool->connect(addr, [url, addr](std::shared_ptr<ddsocket_async> socket) {
        DDASSERT(socket != nullptr);
        std::unique_ptr<ddtls_client> tls_client(new ddtls_client(ddstr::utf8_16(url.m_host)));
        std::shared_ptr<ddtls_socket_async> tls_socket = ddtls_socket_async::create_instance(std::move(tls_client), socket);
        tls_socket->hand_shake([tls_socket, url, addr](bool successful){
            successful;
            auto client = new my_http_client1(tls_socket);
            {
                std::lock_guard<std::recursive_mutex> locker(m_mutex);
                g_clients[client] = std::shared_ptr<my_http_client1>(client);
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

            std::shared_ptr<std::string> content(new std::string());
            *content = request_header.to_str();
            client->get_socket()->send((void*)(*content).data(), (s32)(*content).size(), [client, content](bool successful, u64) {
                if (!successful) {
                    client->on_error();
                }
                client->start_recv();
            });
        });
    });

    return true;
}
DDTEST(test_case_http_clients_async, http_client)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }

    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        DDASSERT(false);
    }

    auto client_pool = ddtcp_connector_async::create_instance(true, *iocp);
    request_https_async(client_pool.get(), "https://www.baidu.com");

    while (!g_stop) {
        if (!iocp->dispatch(1000)) {
            break;
        }
    }
}
} // namespace NSP_DD
