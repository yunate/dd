#include "test/stdafx.h"

#include "ddbase/ddrandom.h"
#include "ddbase/ddtest_case_factory.h"
#include <iostream>

namespace NSP_DD {

DDTEST(test_random, ddrandom)
{
    std::vector<int> ran(100);
    for (size_t i = 0; i < ran.size(); ++i) {
        ddrandom::get_rand_number(0, 10000, ran[i]);
    }

    for (auto it : ran) {
        std::cout << it << std::endl;
    }

    std::string guida;
    ddguid::generate_guid(guida);
    std::cout << guida << std::endl;

    std::wstring guidw;
    ddguid::generate_guid(guidw);
    std::wcout << guidw << std::endl;
}

} // namespace NSP_DD
