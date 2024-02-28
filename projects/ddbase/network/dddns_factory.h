#ifndef ddbase_network_dddns_factory_h_
#define ddbase_network_dddns_factory_h_
#include "ddbase/dddef.h"
#include "ddbase/ddlazy_instance.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/str/ddurl.hpp"
#include "ddbase/thread/ddasync.h"
#include "ddbase/network/ddnetwork_utils.h"

#include <functional>
#include <memory>
#include <mutex>
#include <list>

namespace NSP_DD {

struct dddns_entry
{
    std::string do_main;
    std::vector<std::string> ip_v4s;
    std::vector<std::string> ip_v6s;
    s32 hit_count = 1;
};

class dddns_factory
{
    DDNO_COPY_MOVE(dddns_factory)
    dddns_factory() = default;
public:
    static std::shared_ptr<dddns_factory> create_instance();
    ~dddns_factory();
    void set_cache_max_count(s32 max_count);
    const dddns_entry* get_ip(const std::string& do_main);
    bool get_ip(const std::string& do_main, ddaddr& addr);
    bool get_ip(const ddurl& url, ddaddr& addr);

private:
    dddns_entry* get_from_cache(const std::string& do_main);
    void record_to_cache(dddns_entry* entry);
    dddns_entry* request_dns(const std::string& do_main);

private:
    std::recursive_mutex m_mutex;
    std::list<dddns_entry*> m_cache;
    s32 m_max_count = MAX_S32;

};

} // namespace NSP_DD
#endif // ddbase_network_dddns_factory_h_
