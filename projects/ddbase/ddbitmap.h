#ifndef ddbase_ddbitmap_h_
#define ddbase_ddbitmap_h_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include "ddcolor.h"

namespace NSP_DD {
// 原点坐标在左下角
struct ddbitmap
{
    u32 width = 0;
    u32 height = 0;
    std::vector<ddbgra> colors;

    inline ddbgra& at(s32 x, s32 y)
    {
        return colors[y * width + x];
    }

    inline const ddbgra& at(s32 x, s32 y) const
    {
        return colors[y * width + x];
    }

    // 重置大小,所有数据失效
    void resize(u32 w, u32 h);

    // 从(x, y)出裁剪成长宽为x,y的image
    // 需要调用者满足
    // x: [0, width)
    // y: [0, height)
    // x + w < width
    // y + h < height
    void clip(u32 x, u32 y, u32 w, u32 h);

    // 扩展bitmap, 新扩展的地方使用value复制
    // expend_w/expend_h 需要增加的长宽
    void expend(u32 expend_w, u32 expend_h, const ddbgra& value = { 0, 0, 0, 0 });

    // 翻转
    void flip_h();
    void flip_v();
};

} // namespace NSP_DD
#endif // ddbase_ddbitmap_h_
