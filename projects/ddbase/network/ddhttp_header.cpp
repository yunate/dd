#include "ddbase/stdafx.h"
#include "ddbase/network/ddhttp_header.h"
#include "ddbase/str/ddstr.h"
#include <list>
namespace NSP_DD {
////////////////////////////////////////////////////////ddhttp_header_kvs//////////////////////////////////////////////////////////////////////////////
std::list<std::string>& ddhttp_header_kvs::operator[](const std::string& key)
{
    return m_kvs[key];
}

void ddhttp_header_kvs::set(const std::string& key, const std::string& value)
{
    auto it = m_kvs.find(key);
    if (it == m_kvs.end()) {
        m_kvs[key].push_back(value);
        return;
    }

    if (key == "Content-Length" || key == "Transfer_Encoding") {
        it->second.clear();
    }

    it->second.push_back(value);
}

const std::list<std::string>& ddhttp_header_kvs::get(const std::string& key) const
{
    static std::list<std::string> empty;
    const auto& it = m_kvs.find(key);
    if (it == m_kvs.end()) {
        return empty;
    }
    return it->second;
}

s32 ddhttp_header_kvs::content_lenth() const
{
    const auto& it = get("Content-Length");
    if (it.empty()) {
        return 0;
    }
    return (s32)std::atoi(it.front().c_str());
}

void ddhttp_header_kvs::set_content_lenth(s32 len)
{
    set("Content-Length", ddstr::format("%d", len));
}

std::string ddhttp_header_kvs::transfer_encoding() const
{
    const auto& it = get("Transfer-Encoding");
    if (it.empty()) {
        return "";
    }
    return it.front();
}

void ddhttp_header_kvs::set_transfer_encoding(const std::string& encoding)
{
    set("Transfer-Encoding", encoding);
}

bool ddhttp_header_kvs::is_chunked() const
{
    return (ddstr::lower(transfer_encoding().c_str()).find("chunked") != std::string::npos);
}

void ddhttp_header_kvs::set_chunked()
{
    set_transfer_encoding("chunked");
}

std::string ddhttp_header_kvs::to_str() const
{
    std::string str;
    for (const auto& it : m_kvs) {
        const std::string& key = it.first;
        const auto& values = it.second;
        for (const auto& value : values) {
            str += ddstr::format("%s: %s\r\n", key.c_str(), value.c_str());
        }
    }
    str += "\r\n";
    return str;
}

void ddhttp_header_kvs::clear()
{
    m_kvs.clear();
}

////////////////////////////////////////////////////////ddhttp_request_header//////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////ddhttp_request_header//////////////////////////////////////////////////////////////////////////////
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