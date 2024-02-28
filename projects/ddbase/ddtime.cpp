#include "ddbase/stdafx.h"
#include "ddbase/ddtime.h"
#include "ddbase/str/ddstr.h"
////////////////////////////////////ddtime_period_guard////////////////////////////////////
namespace NSP_DD {
ddtime_period_guard::ddtime_period_guard(u32 period)
    : m_period(period)
{
    // windows下要设置精度，win10 下默认精度是 15ms,也就是说Sleep(1),也睡15ms
    if (::timeBeginPeriod((UINT)m_period) == TIMERR_NOCANDO) {
        m_period = 1;
        (void)::timeBeginPeriod((UINT)m_period);
    }
}

ddtime_period_guard::~ddtime_period_guard()
{
    ::timeEndPeriod((UINT)m_period);
}

void ddsleep(u32 sleep_time, u32 period /* = 1 */)
{
    ddtime_period_guard guard(period);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
}
} // namespace NSP_DD

////////////////////////////////////ddtime////////////////////////////////////
namespace NSP_DD {
bool ddtime::is_leap_year(s32 year)
{
    return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
}

s32 ddtime::day_in_year(s32 year)
{
    return 365 + ((is_leap_year(year)) ? 1 : 0);
}

s32 ddtime::day_in_month(s32 mon, bool leap_year)
{
    DDASSERT(mon < 12);
    DDASSERT(mon >= 0);
    --mon;
    static s32 day[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (mon != 2) {
        return day[mon];
    }
    std::wstring ss;
    return day[mon] + (leap_year ? 1 : 0);
}

ddtime& ddtime::add(s32 add_day)
{
    for (u32 i = 1; i < mon; ++i) {
        add_day += day_in_month(i, is_leap_year(year));
    }
    add_day += day;

    while (add_day > day_in_year(year)) {
        add_day -= day_in_year(year);
        ++year;
    }
    while (add_day <= 0) {
        --year;
        add_day += day_in_year(year);
    }

    mon = 1;
    while (add_day - day_in_month(mon, is_leap_year(year)) > 0) {
        add_day -= day_in_month(mon, is_leap_year(year));
        ++mon;
    }
    day = add_day;
    return *this;
}

s32 ddtime::week_day(s32 year, s32 mon, s32 day)
{
    DDASSERT(mon <= 12);
    DDASSERT(mon > 0);
    static int t[] = { 0,3,2,5,0,3,5,1,4,6,2,4 };
    if (mon < 3) {
        year -= 1;
    }

    s32 wday = (year + year / 4 - year / 100 + year / 400 + t[mon - 1] + day) % 7;
    if (wday == 0) {
        wday = 7;
    }
    return wday;
}

u64 ddtime::now()
{
    static const auto g_system_steady = std::chrono::system_clock::now().time_since_epoch() - std::chrono::steady_clock::now().time_since_epoch();
    return (std::chrono::steady_clock::now().time_since_epoch() + g_system_steady).count();
}

u64 ddtime::now_ms()
{
    auto dur = std::chrono::system_clock::now().time_since_epoch();
    return (u64)std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
}

ddtime ddtime::now_fmt()
{
    return time_fmt(std::chrono::system_clock::now().time_since_epoch().count(), 10000000);
}

ddtime ddtime::ms_fmt(u64 ms_epoch)
{
    return time_fmt(ms_epoch, 1000);
}

ddtime ddtime::micro_fmt(u64 micro_epoch)
{
    return time_fmt(micro_epoch, 1000000);
}

ddtime ddtime::nano_fmt(u64 nano_epoch)
{
    return time_fmt(nano_epoch, 1000000000);
}

ddtime ddtime::time_fmt(u64 epoch, u64 to_sec)
{
    ddtime ti{0};
    time_t tt = epoch / to_sec;
    tm local_tm;
    (void)::localtime_s(&local_tm, &tt);
    ti.year = local_tm.tm_year + 1900;
    ti.mon = local_tm.tm_mon + 1;
    ti.day = local_tm.tm_mday;
    ti.hour = local_tm.tm_hour;
    ti.min = local_tm.tm_min;
    ti.sec = local_tm.tm_sec;
    u64 timeepoch = epoch * (1000000000 / to_sec);
    ti.milli = (timeepoch % 1000000000) / 1000000;
    ti.micro = (timeepoch % 1000000) / 1000;
    ti.nano = (timeepoch % 1000);
    return ti;
}

u64 ddtime::fmt_nano(const ddtime& ti)
{
    tm when{ 0 };
    when.tm_year = ti.year - 1900;
    when.tm_mon = ti.mon - 1;
    when.tm_mday = ti.day;
    when.tm_hour = ti.hour;
    when.tm_min = ti.min;
    when.tm_sec = ti.sec;

    __time64_t s = ::_mktime64(&when);
    if (s == -1) {
        return (u64)0;
    }

    return (u64)(s * 1000000000 + (u64)ti.milli * 1000000 + (u64)ti.micro * 1000 + (u64)ti.nano);
}

const static char* s_week_str[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
const static char* s_month_str[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Otc", "Nov", "Dec" };
std::string ddtime::fmt_gmt(const ddtime& ti)
{
    return ddstr::format("%s, %d %s %d %d:%d:%d GMT", s_week_str[ddtime::week_day(ti.year, ti.mon, ti.day) - 1],
        ti.day, s_month_str[ti.mon - 1], ti.year,
        ti.hour, ti.min, ti.sec);
}

bool ddtime::gmt_fmt(const std::string& gmt, ddtime& ti)
{
    // Sun, 8 Otc 2023 9:40:42 GMT
    if (gmt.size() < 6 || gmt[3] != ',') {
        return false;
    }
    std::string sub = gmt.substr(5);
    std::vector<std::string> out;
    ddstr::split(sub.data(), " ", out);
    if (out.size() != 5) {
        return false;
    }

    ti.day = ::atoi(out[0].c_str());
    if (ti.day == 0) {
        return false;
    }

    for (u32 i = 0; i < 12; ++i) {
        if (out[1] == s_month_str[i]) {
            ti.mon = i + 1;
            break;
        }

        if (i == 11) {
            return false;
        }
    }

    ti.year = ::atoi(out[2].c_str());
    if (ti.year == 0) {
        return false;
    }

    std::vector<std::string> out1;
    ddstr::split(out[3].data(), ":", out1);
    if (out1.size() != 3) {
        return false;
    }

    ti.hour = ::atoi(out1[0].c_str());
    ti.min = ::atoi(out1[1].c_str());
    ti.sec = ::atoi(out1[2].c_str());

    ti.milli = 0;
    ti.micro = 0;
    ti.nano = 0;
    return true;
}

bool operator<(const ddtime& l, const ddtime& r)
{
    if (l.year != r.year) {
        return l.year < r.year;
    }
    if (l.mon != r.mon) {
        return l.mon < r.mon;
    }
    if (l.day != r.day) {
        return l.day < r.day;
    }
    if (l.hour != r.hour) {
        return l.hour < r.hour;
    }
    if (l.min != r.min) {
        return l.min < r.min;
    }
    if (l.sec != r.sec) {
        return l.sec < r.sec;
    }
    if (l.milli != r.milli) {
        return l.milli < r.milli;
    }
    if (l.micro != r.micro) {
        return l.micro < r.micro;
    }
    if (l.nano != r.nano) {
        return l.nano < r.nano;
    }
    return false;
}

bool operator>(const ddtime& l, const ddtime& r)
{
    if (l.year != r.year) {
        return l.year > r.year;
    }
    if (l.mon != r.mon) {
        return l.mon > r.mon;
    }
    if (l.day != r.day) {
        return l.day > r.day;
    }
    if (l.hour != r.hour) {
        return l.hour > r.hour;
    }
    if (l.min != r.min) {
        return l.min > r.min;
    }
    if (l.sec != r.sec) {
        return l.sec > r.sec;
    }
    if (l.milli != r.milli) {
        return l.milli > r.milli;
    }
    if (l.micro != r.micro) {
        return l.micro > r.micro;
    }
    if (l.nano != r.nano) {
        return l.nano > r.nano;
    }
    return false;
}

bool operator<=(const ddtime& l, const ddtime& r)
{
    return !(l > r);
}

bool operator>=(const ddtime& l, const ddtime& r)
{
    return !(l < r);
}

bool operator==(const ddtime& l, const ddtime& r)
{
    if (l.year == r.year
        && l.mon == r.mon
        && l.day == r.day
        && l.hour == r.hour
        && l.min == r.min
        && l.sec == r.sec
        && l.milli == r.milli
        && l.micro == r.micro
        && l.nano == r.nano) {
        return true;
    }

    return false;
}
} // namespace NSP_DD

////////////////////////////////////ddexpire////////////////////////////////////
namespace NSP_DD {
ddexpire ddexpire::never;

ddexpire ddexpire::form_timeout(u32 timeout)
{
    if (timeout == INFINITE) {
        return never;
    }
    u64 now = ddtime::now_ms();
    if (never.epoch - now <= timeout) {
        return never;
    }
    return { now + timeout };
}

s32 ddexpire::get_timeout()
{
    if (*this == never) {
        return INFINITE;
    }

    u64 now = ddtime::now_ms();
    if (now < epoch) {
        return s32(epoch - now);
    }
    return 0;
}
} // namespace NSP_DD
