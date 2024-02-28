
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

bool send_file(const std::wstring& src_path, const std::wstring& dst_path)
{
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
    request_header.uri = "chunked";
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
    request_header.kvs.set_transfer_encoding("chunked");
    if (!client->send_header(request_header)) {
        return false;
    }

    // send chunked data
    s32 send_size = 0;
    auto file_stream = ddfile_stream::create_instance(src_path);
    s32 size = (s32)file_stream->size();
    file_stream->seek(0, SEEK_SET);
    while (size > 0) {
        ddmemory_stream chunked;
        u8 file_buff[1024];
        s32 readed = file_stream->read(file_buff, 1024);
        std::string chunken_len_str = ddstr::format("%x\r\n", readed);
        chunked.write((u8*)chunken_len_str.data(), (s32)chunken_len_str.size());
        chunked.write(file_buff, readed);
        chunked.write((u8*)"\r\n", 2);
        chunked.seek(0, SEEK_SET);
        if (!client->send_body(&chunked)) {
            return false;
        }
        size -= readed;
        send_size += readed;
    }

    std::string chunk_end = "0\r\n\r\n";
    if (!client->send_body((u8*)(chunk_end).data(), (s32)(chunk_end).size())) {
        return false;
    }

    std::shared_ptr<ddfile> file(ddfile::create_utf8_file(dst_path));
    file->resize(0);

    s32 dst_size = 0;
    while (true) {
        const auto* it = client->recv();
        if (it == nullptr || it->parse_result.parse_state == dddata_parse_state::error) {
            return false;
        }

        if (it->parse_result.parse_state == dddata_parse_state::complete) {
            // successful
            if (dst_size != file_stream->size()) {
                return false;
            }
            break;
        }

        if (it->body_buff != nullptr && it->body_buff_size > 0) {
            dst_size += it->body_buff_size;
            if (file->write(it->body_buff, it->body_buff_size) != it->body_buff_size) {
                return false;
            }
            //std::cout << it->body_buff_size << " ";
        }
    }


    std::cout << std::endl;
    if (send_size != file_stream->size()) {
        return false;
    }
    return true;
}

DDTEST(test_case_http_chunked_client, tcp_server)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }

    ddtimer time;
    std::wstring src_path = LR"(F:\My\test_folder\test_http_chunked\index.html)";
    std::wstring dst_path_base = LR"(F:\My\test_folder\test_http_chunked\)";
    const s32 all_test_count = 10000;
    const s32 thread_count = 8;
    std::thread* t[thread_count];
    for (int j = 0; j < thread_count; ++j) {
        t[j] = new std::thread([&dst_path_base, &src_path, j, thread_count, all_test_count]() {
            for (s32 i = 0; i < all_test_count / thread_count; ++i) {
                std::wstring dst_path = ddpath::join(dst_path_base, ddstr::format(L"index%d_%d.html", j, i));
                if (!send_file(src_path, dst_path)) {
                    int k = 0;
                    ++k;
                }
            }

            return true;
        });
    }

    for (int i = 0; i < thread_count; ++i) {
        t[i]->join();
        delete t[i];
    }

    //for (s32 i = 0; i < 10000; ++i) {
    //    std::wstring dst_path = ddpath::join(dst_path_base, ddstr::format(L"index%d.html", i));
    //    if (!test_client_inner(src_path, dst_path)) {
    //        int k = 0;
    //        ++k;
    //    }
    //}

    std::cout << time.get_time_pass() / 1000 / 1000;
}

} // namespace NSP_DD
