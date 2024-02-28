#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"
#include "ddbase/network/http/ddhttp_header.h"

////////////////////////////////////////////////////////ddhttp_header_kvs//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
static const char* k_content_length = "content-length";
static const char* k_transfer_encoding = "transfer-encoding";
static const char* k_chunked = "chunked";
static const char* k_connection = "connection";
static const char* k_keep_alive = "keep-alive";

static bool ddstrkey_cmp_no_case(const char* l, const char* r)
{
    while (std::tolower(*l) == std::tolower(*r)) {
        if (0 == *l) {
            return true;
        }

        ++l;
        ++r;
    }

    return false;
}

// Multiple occurances of some headers cannot be coalesced into a comma-
// separated list since their values are (or contain) unquoted HTTP-date
// values, which may contain a comma (see RFC 2616 section 3.3.1).
static bool ddsupport_coalescing_headers(const std::string& key) {
    static constexpr char* k_not_support_coalescing_headers[] = {
        "date",
        "expires",
        "last-modified",
        "location",
        "retry-after",
        "set-cookie",
        // The format of auth-challenges mixes both space separated tokens and
        // comma separated properties, so coalescing on comma won't work.
        "www-authenticate",
        "proxy-authenticate",
        // STS specifies that UAs must not process any STS headers after the first one.
        "strict-transport-security"
    };

    for (char* header : k_not_support_coalescing_headers) {
        if (ddstrkey_cmp_no_case(header, key.c_str())) {
            return false;
        }
    }
    return true;
}

void ddhttp_header_kvs::append(const std::string& key, const std::string& value)
{
    if (ddsupport_coalescing_headers(key)) {
        for (auto& it : m_kvs) {
            if (ddstrkey_cmp_no_case(it.first.c_str(), key.c_str())) {
                std::string& it_value = it.second;
                it_value += ", ";
                it_value += value;
                return;
            }
        }
    }

    m_kvs.push_back({ key, value });
}

void ddhttp_header_kvs::reset(const std::string& key, const std::string& value)
{
    remove(key);
    m_kvs.push_back({ key, value });
}

void ddhttp_header_kvs::get(const std::string& key, std::vector<std::string>& result) const
{
    result.clear();
    for (const auto& it : m_kvs) {
        if (ddstrkey_cmp_no_case(it.first.c_str(), key.c_str())) {
            result.push_back(it.second);
        }
    }
#ifdef _DEBUG
    if (ddsupport_coalescing_headers(key)) {
        DDASSERT(result.size() <= 1);
    }
#endif
}

void ddhttp_header_kvs::remove(const std::string& key)
{
    (void)std::erase_if(m_kvs, [&key](const auto& it) {
        return ddstrkey_cmp_no_case(key.c_str(), it.first.c_str());
    });
}

s32 ddhttp_header_kvs::content_lenth() const
{
    std::vector<std::string> result;
    get(k_content_length, result);
    if (result.empty()) {
        return 0;
    }
    return (s32)std::atoi(result.front().c_str());
}

void ddhttp_header_kvs::set_content_lenth(s32 len)
{
    reset(k_content_length, ddstr::format("%d", len));
}

std::string ddhttp_header_kvs::transfer_encoding() const
{
    std::vector<std::string> result;
    get(k_transfer_encoding, result);
    if (result.empty()) {
        return "";
    }
    return result.front();
}

void ddhttp_header_kvs::set_transfer_encoding(const std::string& encoding)
{
    reset(k_transfer_encoding, encoding);
}

bool ddhttp_header_kvs::is_chunked() const
{
    return (ddstr::lower(transfer_encoding().c_str()).find(k_chunked) != std::string::npos);
}

void ddhttp_header_kvs::set_chunked()
{
    set_transfer_encoding(k_chunked);
}

bool ddhttp_header_kvs::keep_alive() const
{
    std::vector<std::string> result;
    get(k_connection, result);
    for (const auto& it : result) {
        if (ddstrkey_cmp_no_case(it.c_str(), k_keep_alive)) {
            return true;
        }
    }

    return false;
}

void ddhttp_header_kvs::set_keep_alive(bool keep_alive)
{
    remove(k_connection);
    if (keep_alive) {
        m_kvs.push_back({ k_connection, k_keep_alive });
    }
}

std::string ddhttp_header_kvs::to_str() const
{
    std::string str;
    for (const auto& it : m_kvs) {
        str += ddstr::format("%s: %s\r\n", it.first.c_str(), it.second.c_str());
    }
    str += "\r\n";
    return str;
}

void ddhttp_header_kvs::clear()
{
    m_kvs.clear();
}
} // namespace NSP_DD

////////////////////////////////////////////////////////ddhttp_request_header//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
void ddhttp_request_header::as_default(const ddurl& url)
{
    as_default(url.host);
    uri = url.path;
}

void ddhttp_request_header::as_default(const std::string& do_main)
{
    if (uri.empty()) {
        uri = "/";
    }

    kvs.clear();
    kvs.reset("Host", do_main);
    kvs.reset("Accept", "*/*");
    kvs.reset("Accept-Language", "zh-CN, zh;q=0.9, en;q=0.8, en-GB;q=0.7, en-US;q=0.6");
    kvs.reset("Accept-Encoding", "gzip, compress, deflate, br, zstd, identity");
    kvs.reset("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36 Edg/118.0.2088.57");
}

std::string ddhttp_request_header::to_str() const
{
    std::string str;
    str += ddstr::format("%s %s %s\r\n", method.c_str(), uri.c_str(), version.c_str());
    str += kvs.to_str();
    return str;
}

void ddhttp_request_header::reset()
{
    method = "GET";
    uri = "/";
    version = "HTTP/1.1";
    kvs.clear();
}
} // namespace NSP_DD

////////////////////////////////////////////////////////ddhttp_response_header//////////////////////////////////////////////////////////////////////////////
namespace NSP_DD {
std::string ddhttp_response_header::to_str() const
{
    std::string str;
    str += ddstr::format("%s %d %s\r\n", version.c_str(), state, state_str.c_str());
    str += kvs.to_str();
    return str;
}

void ddhttp_response_header::reset()
{
    version = "HTTP/1.1";
    state = 200;
    state_str = "OK";
    kvs.clear();
}
} // namespace NSP_DD
