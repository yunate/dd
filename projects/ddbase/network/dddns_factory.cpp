#include "ddbase/stdafx.h"
#include "ddbase/network/dddns_factory.h"
#include "ddbase/network/ddnet_util.h"
#include "ddbase/network/ddnetwork_async_caller.hpp"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
dddns_factory::dddns_factory(const ddasync_caller& async_caller) :
    m_async_caller(async_caller)
{
}

std::shared_ptr<dddns_factory> dddns_factory::create_instance(const ddasync_caller& async_caller)
{
    dddns_factory* factory = new(std::nothrow) dddns_factory(async_caller);
    if (factory == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }
    return std::shared_ptr<dddns_factory>(factory);
}

dddns_factory::~dddns_factory()
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    for (auto& it : m_catch) {
        delete it;
    }
    m_catch.clear();
}

const dddns_entry* dddns_factory::get_ip_sync(const std::string& do_main)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    dddns_entry* entry = get_from_catch(do_main);
    if (entry != nullptr) {
        return entry;
    }

    return request_dns(do_main);
}

void dddns_factory::get_ip(const std::string& do_main, const std::function<void(const dddns_entry* entry)>& callback)
{
    if (callback == nullptr) {
        return;
    }

    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    dddns_entry* entry = get_from_catch(do_main);
    if (entry != nullptr) {
        callback(entry);
        return;
    }

    std::weak_ptr<dddns_factory> weak = weak_from_this();
    ddmaybe_async_call(m_async_caller, [weak, this, callback, do_main]() {
        auto it = weak.lock();
        if (it != nullptr) {
            callback(request_dns(do_main));
        }
    });
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
        ddnet_utils::sockaddr_to_ddaddr(it->ai_addr, addr);
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
        record_to_catch(entry);
    }
    return entry;
}

void dddns_factory::set_catch_max_count(s32 max_count)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    m_max_count = max_count;
}

dddns_entry* dddns_factory::get_from_catch(const std::string& do_main)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    for (const auto& it : m_catch) {
        if (it->do_main == do_main) {
            ++it->hit_count;
            return it;
        }
    }
    return nullptr;
}

void dddns_factory::record_to_catch(dddns_entry* entry)
{
    std::lock_guard<std::recursive_mutex> guard(m_mutex);
    DDASSERT(entry != nullptr);
    auto min_count_it = m_catch.begin();
    for (auto it = m_catch.begin(); it != m_catch.end(); ++it) {
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

    if (m_max_count <= (s32)m_catch.size()) {
        m_catch.erase(min_count_it);
        delete (*min_count_it);
    }

    m_catch.push_front(entry);
}

} // namespace NSP_DD
