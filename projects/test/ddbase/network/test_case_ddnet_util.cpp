#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/network/ddnet_util.h"
#include "ddbase/str/ddstr.h"

#include <ws2tcpip.h>
#include <ip2string.h>
#include <iostream>

namespace NSP_DD {
DDTEST(test_case_ddnet_util, addr_s_i)
{
    struct test_item
    {
        std::string saddr;
        bool result = false;
    };

    // ipv4
    {
        std::vector<test_item> test_items =
        {
            {"128.0.0.1", true},
            {"256.0.0.1", false},
            {"128.0.0.1.1", false},
            {"128.0.0.1.", false},
            {"128.0.0.", false},
            {"128.0.0.", false}
        };

        for (const auto& it : test_items) {
            in_addr target{0};
            bool b = ddnet_utils::ddaddr_to_sockaddr(ddaddr{ it.saddr, 8888, true }, (::sockaddr*)&target);
            DDASSERT(b == it.result);

            if (b) {
                ddaddr add;
                bool b1 = ddnet_utils::sockaddr_to_ddaddr((::sockaddr*)&target, add);
                DDASSERT(b1 == it.result);
                DDASSERT(add.ip == it.saddr);
            }
        }
    }

    // ipv6
    {
        std::vector<test_item> test_items =
        {
            {"2345:1425:2CA1:1234:1234:1567:5673:23b5", true},
            {"2345:1425:2CA1:1234:1234:1567:5673:23b", true},
            {"2345:1425:2CA1:1234:1234:1567:5673:23b55", false},
            {"", false},
        };

        for (const auto& it : test_items) {
            in6_addr target{ 0 };
            bool b = ddnet_utils::ddaddr_to_sockaddr(ddaddr{ it.saddr, 8888, false }, (::sockaddr*)&target);
            DDASSERT(b == it.result);

            if (b) {
                ddaddr add;
                bool b1 = ddnet_utils::sockaddr_to_ddaddr((::sockaddr*)&target, add);
                DDASSERT(b1 == it.result);
                // DDASSERT(add.ip == it.saddr);
            }
        }
    }
}

} // namespace NSP_DD
