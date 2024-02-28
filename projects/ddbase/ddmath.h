#ifndef ddbase_ddmath_h_
#define ddbase_ddmath_h_
#include "ddbase/dddef.h"
#include <stdint.h>
namespace NSP_DD {

// 32位有符号数最大/小值
#define MAX_S32 INT32_MAX
#define MIN_S32 INT32_MIN

// 32位无符号数最大值
#define MAX_U32 UINT32_MAX

// 64位有符号数最大/小值
#define MAX_S64 INT64_MAX
#define MIN_S64 INT64_MIN

// 64位无符号数最大值
#define MAX_U64 UINT64_MAX

// 取64位高位
#define L64(l) ((__int64)(l) & 0xffffffff)

// 取64位低位d
#define H64(l) (((__int64)(l) >> 32) & 0xffffffff)

// 计算a 按照b 字节对齐后的结果
// 计算b倍数大于等于a的最小上界
template<class T, class U>
inline T align(T a, U b)
{
    return (a + b - 1) & ~(b - 1);
}

struct ddpoint
{
    s32 x;
    s32 y;

    ddpoint operator +(const ddpoint& r) const
    {
        return ddpoint{x + r.x, y + r.y};
    }

    void operator +=(const ddpoint& r)
    {
        x += r.x;
        y += r.y;
    }

    ddpoint operator -(const ddpoint& r) const
    {
        return ddpoint{ x - r.x, y - r.y };
    }

    void operator -=(const ddpoint& r)
    {
        x -= r.x;
        y -= r.y;
    }
};

struct ddrect
{
    s32 l; // left
    s32 r; // right
    s32 t; // top
    s32 b; // bottom

    ddrect operator +(const ddpoint& target) const
    {
        return ddrect{ l + target.x, r + target.x, t + target.y, b + target.y };
    }

    void operator +=(const ddpoint& target)
    {
        l += target.x;
        r += target.x;
        t += target.y;
        b += target.y;
    }

    ddrect operator -(const ddpoint& target) const
    {
        return ddrect{ l - target.x, r - target.x, t - target.y, b - target.y };
    }

    void operator -=(const ddpoint& target)
    {
        l -= target.x;
        r -= target.x;
        t -= target.y;
        b -= target.y;
    }

    ddpoint pos()
    {
        return ddpoint{ l, t };
    }

    ddpoint size()
    {
        return ddpoint{ r - l, b - t };
    }

    // 是否包含点
    inline bool is_contian_point(const ddpoint& pt) const
    {
        if (pt.x >= l && pt.x <= r && pt.y >= t && pt.y <= b) {
            return true;
        }
        return false;
    }

    // 是否与另外一个rect相交
    inline bool is_rect_overlap(const ddrect& target) const
    {
        if (l > target.r || r < target.l || t < target.b || b > target.t) {
            return false;
        }
        return true;
    }
};

} // namespace NSP_DD
#endif // ddbase_ddmath_h_

