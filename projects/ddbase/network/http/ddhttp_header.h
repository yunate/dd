#ifndef ddbase_network_http_ddhttp_header_h_
#define ddbase_network_http_ddhttp_header_h_
#include "ddbase/dddef.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include "ddbase/network/ddnetwork_utils.h"
#include "ddbase/stream/ddistream.h"
#include "ddbase/str/ddurl.hpp"
#include "ddbase/str/ddstr.h"
#include <memory>
#include <map>
#include <list>
namespace NSP_DD {
class ddhttp_header_kvs
{
public:
    void append(const std::string& key, const std::string& value);
    void reset(const std::string& key, const std::string& value);
    void get(const std::string& key, std::vector<std::string>& result) const;
    void remove(const std::string& key);
    std::string to_str() const;
    void clear();

    // Content-Length
    s32 content_lenth() const;
    void set_content_lenth(s32 len);

    // Transfer-Encoding
    std::string transfer_encoding() const;
    void set_transfer_encoding(const std::string& encoding);

    // chunked, "Transfer-Encoding" = "chunked"
    bool is_chunked() const;
    void set_chunked();

    bool keep_alive() const;
    void set_keep_alive(bool keep_alive);

private:
    std::list<std::pair<std::string, std::string>> m_kvs;
};

struct ddhttp_request_header
{
    void as_default(const ddurl& url);
    void as_default(const std::string& do_main);

    std::string to_str() const;
    void reset();

    // GET POST SET DELETE
    std::string method = "GET";
    std::string uri = "/";
    std::string version = "HTTP/1.1";
    ddhttp_header_kvs kvs;
};

struct ddhttp_response_header
{
    std::string to_str() const;
    void reset();

    std::string version = "HTTP/1.1";
    s32 state = 200;
    std::string state_str = "OK";
    ddhttp_header_kvs kvs;
};

} // namespace NSP_DD
#endif // ddbase_network_http_ddhttp_header_h_