#ifndef ddbase_network_dddns_factory_h_
#define ddbase_network_dddns_factory_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddhttp_header.h"
#include "ddbase/network/ddnet_util.h"
#include "ddbase/network/ddtcp_connector_sync.h"
#include "ddbase/ddlazy_instance.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/thread/ddasync.h"

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

class dddns_factory : std::enable_shared_from_this<dddns_factory>
{
    DDNO_COPY_MOVE(dddns_factory)
    dddns_factory(const ddasync_caller& async_caller);
public:
    static std::shared_ptr<dddns_factory> create_instance(const ddasync_caller& async_caller);
    ~dddns_factory();
    void get_ip(const std::string& do_main, const std::function<void(const dddns_entry* entry)>& callback);
    const dddns_factory::dddns_entry* get_ip_sync(const std::string& do_main);
    void set_catch_max_count(s32 max_count);

private:
    dddns_entry* get_from_catch(const std::string& do_main);
    void record_to_catch(dddns_entry* entry);
    dddns_entry* request_dns(const std::string& do_main);

private:
    std::recursive_mutex m_mutex;
    ddasync_caller m_async_caller;
    std::list<dddns_entry*> m_catch;
    s32 m_max_count = MAX_S32;
};

} // namespace NSP_DD
#endif // ddbase_network_dddns_factory_h_
