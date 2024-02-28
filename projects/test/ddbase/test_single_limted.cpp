
#include "test/stdafx.h"
#include "ddbase/ddsingle_limited.h"

#include "ddbase/ddtest_case_factory.h"

#include <iostream>

namespace NSP_DD {

DDTEST(test_single_limted, auto_limted)
{
    {
        ddsingle_limited limted;

        if (limted.try_hold_mutex(L"limted")) {
            std::cout << "holded" << std::endl;
        }

        if (!limted.try_hold_mutex(L"limted")) {
            std::cout << "not holded" << std::endl;
        }
    }

    {
        ddsingle_limited limted;

        if (limted.try_hold_mutex(L"limted")) {
            std::cout << "holded" << std::endl;
        }

        if (!limted.try_hold_mutex(L"limted")) {
            std::cout << "not holded" << std::endl;
        }
    }
}
} // namespace NSP_DD
