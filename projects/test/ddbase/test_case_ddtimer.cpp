#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddtime.h"
#include "ddbase/ddassert.h"

#include <iostream>
#include <thread>

namespace NSP_DD {
DDTEST(test_xtimer, get_time_pass)
{
    ddtimer timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    timer.get_time_pass();
    std::cout << timer.get_time_pass() / 1000 / 1000 << std::endl;
}

DDTEST(test_xtimer1, ti_s)
{
    ddtime ti{2023, 01, 28, 11, 19, 00};
    u64 epoch = ddtime::fmt_nano(ti);
    DDASSERT(epoch != 0);
    ddtime ti1 = ddtime::ms_fmt(epoch);
    DDASSERT(ti1 == ti);
}

DDTEST(test_xtimer1, is_leap_year)
{
    DDASSERT(ddtime::is_leap_year(1900) == false);
    DDASSERT(ddtime::is_leap_year(2000) == true);
    DDASSERT(ddtime::is_leap_year(1998) == false);
    DDASSERT(ddtime::week_day(2023, 01, 23) == 1);
    DDASSERT(ddtime::week_day(2023, 01, 24) == 2);
    DDASSERT(ddtime::week_day(2023, 01, 25) == 3);
    DDASSERT(ddtime::week_day(2023, 01, 26) == 4);
    DDASSERT(ddtime::week_day(2023, 01, 27) == 5);
    DDASSERT(ddtime::week_day(2023, 01, 28) == 6);
    DDASSERT(ddtime::week_day(2023, 01, 29) == 7);
}

DDTEST(test_xtimer1, add)
{
    {
        ddtime ti{ 2023, 01, 28, 11, 19, 00 };
        ti.add(100);
        DDASSERT(ti.year == 2023);
        DDASSERT(ti.mon == 5);
        DDASSERT(ti.day == 8);
    }

    {
        ddtime ti{ 2023, 01, 28, 11, 19, 00 };
        ti.add(300);
        DDASSERT(ti.year == 2023);
        DDASSERT(ti.mon == 11);
        DDASSERT(ti.day == 24);
    }

    {
        ddtime ti{ 2023, 01, 28, 11, 19, 00 };
        ti.add(-102);
        DDASSERT(ti.year == 2022);
        DDASSERT(ti.mon == 10);
        DDASSERT(ti.day == 18);
    }

    {
        ddtime ti{ 2023, 01, 28, 11, 19, 00 };
        ti.add(-1000);
        DDASSERT(ti.year == 2020);
        DDASSERT(ti.mon == 5);
        DDASSERT(ti.day == 3);
    }

    {
        ddtime ti{ 2023, 01, 28, 11, 19, 00 };
        ti.add(1000);
        DDASSERT(ti.year == 2025);
        DDASSERT(ti.mon == 10);
        DDASSERT(ti.day == 24);
    }
}

DDTEST(test_xtimer2, GMT)
{
    ddtime now = ddtime::now_fmt();
    std::string fmt_str = ddtime::fmt_gmt(now);
    ddtime now1;
    ddtime::gmt_fmt(fmt_str, now1);
}

} // namespace NSP_DD
