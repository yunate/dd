#ifndef ddbase_network_ddhttp_header_h_
#define ddbase_network_ddhttp_header_h_
#include "ddbase/dddef.h"
#include <map>
#include <list>

namespace NSP_DD {
class ddhttp_header_kvs
{
public:
    std::list<std::string>& operator[](const std::string& key);
    void set(const std::string& key, const std::string& value);
    const std::list<std::string>& get(const std::string& key) const;
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

private:
    std::map<std::string, std::list<std::string>> m_kvs;
};

/* http request header
* method uri version\r\n
* key:value\r\n
* key:value\r\n
* key:value\r\n
* ...
* key:value\r\n
* \r\n
* body
*/
struct ddhttp_request_header
{
    std::string to_str() const;
    void reset();

    // GET POST SET DELETE
    std::string method = "GET";
    std::string uri = "/";
    std::string version = "HTTP/1.1";
    ddhttp_header_kvs kvs;
};

/* http response header
* version state state_str\r\n
* key:value\r\n
* key:value\r\n
* key:value\r\n
* ...
* key:value\r\n
* \r\n
* body
*/
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
#endif // ddbase_network_ddhttp_header_h_
