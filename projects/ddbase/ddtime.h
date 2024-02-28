
#ifndef ddbase_ddtime_h_
#define ddbase_ddtime_h_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include <chrono>
#include <thread>

#pragma comment(lib, "Winmm.lib")
#include <windows.h>

namespace NSP_DD {
////////////////////////////////////ddtime_period_guard////////////////////////////////////
class ddtime_period_guard
{
public:
    ddtime_period_guard(u32 period);
    ~ddtime_period_guard();

private:
    u32 m_period = 0;
};

// sleep_time 单位毫秒
// period 精度, sleep_time 向上对齐 period
void ddsleep(u32 sleep_time, u32 period = 1);
} // namespace NSP_DD

////////////////////////////////////ddtime////////////////////////////////////
namespace NSP_DD {
struct ddtime
{
    // 年月日时分秒
    u32 year;
    u32 mon;        // [1-12]
    u32 day;        // [1-31]
    u32 hour;       // [0-23]
    u32 min;        // [0-59]
    u32 sec;        // [0-60]
    u32 milli;      // [0-999]
    u32 micro;      // [0-999999]
    u32 nano;       // [0-999999999]

    // add_day天后
    ddtime& add(s32 add_day);

    // 是否是闰年
    static bool is_leap_year(s32 year);

    // 某一年有多少天
    static s32 day_in_year(s32 year);

    // 某一月[1-12]有多少天
    static s32 day_in_month(s32 mon, bool leap_year);

    // 星期几, 返回[1-7]
    // mon [1-12]
    static s32 week_day(s32 year, s32 mon, s32 day);

    // 获得时间戳 纳秒
    static u64 now();

    // 获得时间戳 毫秒
    static u64 now_ms();

    // 时间戳 -> ddtime
    static ddtime now_fmt();
    static ddtime ms_fmt(u64 ms_epoch);
    static ddtime micro_fmt(u64 micro_epoch);
    static ddtime nano_fmt(u64 nano_epoch);
    static ddtime time_fmt(u64 epoch, u64 to_sec);

    // ddtime -> 时间戳
    static u64 fmt_nano(const ddtime& ti);

    // ddtime -> GMT(格林威治标准时间)字符串
    // Sun, 8 Otc 2023 9:40:42 GMT
    static std::string fmt_gmt(const ddtime& ti);
    static bool gmt_fmt(const std::string& gmt, ddtime& ti);
};

bool operator<(const ddtime& l, const ddtime& r);
bool operator>(const ddtime& l, const ddtime& r);
bool operator<=(const ddtime& l, const ddtime& r);
bool operator>=(const ddtime& l, const ddtime& r);
bool operator==(const ddtime& l, const ddtime& r);
} // namespace NSP_DD

////////////////////////////////////ddtimer////////////////////////////////////
namespace NSP_DD {
class ddtimer
{
public:
    ddtimer()
    {
        reset();
    }

    ~ddtimer() = default;

    void reset()
    {
        m_epoch = ddtime::now();
    }

    u64 get_time_pass()
    {
        return ddtime::now() - m_epoch;
    }

private:
    // 时间戳
    u64 m_epoch = 0;
};
} // namespace NSP_DD

////////////////////////////////////ddexpire////////////////////////////////////
// 到期时间(毫秒)
namespace NSP_DD {
struct ddexpire
{
    u64 epoch = (u64)(-1);
    static ddexpire never;
    static ddexpire form_timeout(u32 timeout);
    s32 get_timeout();
};
inline bool operator==(const ddexpire& l, const ddexpire& r)
{
    return l.epoch == r.epoch;
}
} // namespace NSP_DD
#endif // ddbase_ddtime_h_
