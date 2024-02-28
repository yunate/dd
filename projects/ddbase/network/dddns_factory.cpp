#include "ddbase/stdafx.h"
#include "ddbase/network/dddns_factory.h"
#include "ddbase/network/ddnetwork_utils.h"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
std::shared_ptr<dddns_factory> dddns_factory::create_instance()
{

    std::shared_ptr<dddns_factory> inst(new(std::nothrow)dddns_factory());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return  nullptr;
    }

    return inst;
}

dddns_factory::~dddns_factory()
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    for (auto& it : m_cache) {
        delete it;
    }
    m_cache.clear();
}

const dddns_entry* dddns_factory::get_ip(const std::string& do_main)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    dddns_entry* entry = get_from_cache(do_main);
    if (entry != nullptr) {
        return entry;
    }

    return request_dns(do_main);
}

bool dddns_factory::get_ip(const std::string& do_main, ddaddr& addr)
{
    const dddns_entry* dns_entry = get_ip(do_main);
    if (dns_entry == nullptr) {
        return false;
    }

    if (!dns_entry->ip_v4s.empty()) {
        addr.ip = dns_entry->ip_v4s[0];
        addr.ipv4_6 = true;
    } else if (!dns_entry->ip_v6s.empty()) {
        addr.ip = dns_entry->ip_v6s[0];
        addr.ipv4_6 = false;
    }

    return true;
}

bool dddns_factory::get_ip(const ddurl& url, ddaddr& addr)
{
    return get_ip(url.host, addr);
}

dddns_entry* dddns_factory::request_dns(const std::string& do_main)
{
    ::addrinfo hints;
    ::ZeroMemory(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    ::addrinfo* result = nullptr;
    if (::getaddrinfo(do_main.c_str(), nullptr, &hints, &result) != 0) {
        return nullptr;
    }

    dddns_entry* entry = new dddns_entry();
    entry->do_main = do_main;
    for (::addrinfo* it = result; it != nullptr; it = it->ai_next) {
        ddaddr addr;
        addr.from_sockaddr(it->ai_addr);
        if (addr.ipv4_6) {
            entry->ip_v4s.push_back(addr.ip);
        } else {
            entry->ip_v6s.push_back(addr.ip);
        }
    }
    ::freeaddrinfo(result);

    if (entry->ip_v4s.empty() && entry->ip_v6s.empty()) {
        delete entry;
        return nullptr;
    } else {
        record_to_cache(entry);
    }
    return entry;
}

void dddns_factory::set_cache_max_count(s32 max_count)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_max_count = max_count;
}

dddns_entry* dddns_factory::get_from_cache(const std::string& do_main)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    for (const auto& it : m_cache) {
        if (it->do_main == do_main) {
            ++it->hit_count;
            return it;
        }
    }
    return nullptr;
}

void dddns_factory::record_to_cache(dddns_entry* entry)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    DDASSERT(entry != nullptr);
    auto min_count_it = m_cache.begin();
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
        if (entry->do_main == (*it)->do_main) {
            delete (*it);
            ++entry->hit_count;
            (*it) = entry;
            return;
        }

        if ((*it)->hit_count < (*min_count_it)->hit_count) {
            min_count_it = it;
        }
    }

    if (m_max_count <= (s32)m_cache.size()) {
        m_cache.erase(min_count_it);
        delete (*min_count_it);
    }

    m_cache.push_front(entry);
}

} // namespace NSP_DD
