
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/network/ddnetwork.h"

namespace NSP_DD {

void get_ip(const std::string& do_main)
{
    DDLAZY_INSTANCE(dddns_factory).get_ip(do_main, [do_main](const dddns_entry* entry) {
        std::cout << "do_main" << std::endl;
        if (entry == nullptr) {
            std::cout << "  can not get the ip" << std::endl;
            return;
        }

        for (const auto& it : entry->ip_v4s) {
            std::cout << "  " << it << std::endl;
        }

        for (const auto& it : entry->ip_v6s) {
            std::cout << "  " << it << std::endl;
        }
    });
}

DDTEST(test_case_dddns_factory, ddiocp_socket_server)
{
    ddnetwork_initor initor;
    if (!initor.init(nullptr)) {
        DDASSERT(false);
    }

    //for (s32 i = 0; i < 100; ++i) {
    //    get_ip("wwww.baidu.com");
    //}

    for (s32 i = 0; i < 100; ++i) {
        auto it = DDLAZY_INSTANCE(dddns_factory).get_ip_sync("wwww.baidu.com");
        for (const auto& its : it->ip_v4s) {
            std::cout << "  " << its << std::endl;
        }

        for (const auto& its : it->ip_v6s) {
            std::cout << "  " << its << std::endl;
        }
    }

    auto it = DDLAZY_INSTANCE(dddns_factory).get_ip_sync("wwww.baidu.com");
    for (const auto& its : it->ip_v4s) {
        std::cout << "  " << its << std::endl;
    }

    for (const auto& its : it->ip_v6s) {
        std::cout << "  " << its << std::endl;
    }
    while (true) {
        ::Sleep(1000);
    }
}

} // namespace NSP_DD