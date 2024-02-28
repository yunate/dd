#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddfilter.h"

#include <iostream>
#include <functional>

namespace NSP_DD {

int Fun(int& x, int)
{
    std::cout << "int Fun(int, int)" << std::endl;
    x = 2;
    return 1;
}

int FunFilter(int&, int)
{
    std::cout << "int FunFilter(int, int)" << std::endl;
    return 1;
}

DDTEST(test_ddfilter, create_cluser_with_filter1)
{
    auto it = create_ddcluser_with_filter(Fun, FunFilter);
    int x = 0;
    it(x, 1);
}

int FunFilter2(int, int)
{
    std::cout << "int FunFilter2(int, int)" << std::endl;
    return 1;
}

DDTEST(test_ddfilter, create_ddcluser_with_filter)
{
    auto callable = [](int& x, int) {
        std::cout << "lambda callable" << std::endl;
        x = 3;
        return 1;
    };
    
    auto filter = [](int, int) {
        std::cout << "lambda filter" << std::endl;
        return true;
    };
    auto it = create_ddcluser_with_filter(callable, filter);
    int x = 0;
    it(x, 1);
    // it(1, 1);
}

} // namespace NSP_DD