#include "ddbase/stdafx.h"
#include "ddbase/ddbitmap.h"
namespace NSP_DD {

void ddbitmap::resize(u32 w, u32 h)
{
    colors.resize(w * h);
    width = w;
    height = h;
}

void ddbitmap::clip(u32 x, u32 y, u32 w, u32 h)
{
    DDASSERT(x < width);
    DDASSERT(y < height);
    DDASSERT(x + w < width);
    DDASSERT(y + h < height);

    for (u32 y1 = 0; y1 < h; ++y1) {
        for (u32 x1 = 0; x1 < w; ++x1) {
            DDBITMAP_AT(x1, y1) = DDBITMAP_AT(x1 + x, y1 + y);
        }
    }
    width = w;
    height = h;
    colors.resize(w * h);
}

void ddbitmap::expend(u32 expend_w, u32 expend_h, const ddbgra& value)
{
    resize(width + expend_w, height + expend_h);
    for (u32 y = 0; y < height - expend_h; ++y) {
        for (u32 x = 0; x < width - expend_w; ++x) {
            DDBITMAP_AT(x, y) = DDBITMAP_AT1(x, y, (width - expend_w));;
        }
    }

    // 填充默认值
    if (!(value == ddbgra{0, 0, 0, 0})) {
        for (u32 y = height - expend_h; y < height; ++y) {
            for (u32 x = width - expend_w; x < width; ++x) {
                DDBITMAP_AT(x, y) = value;
            }
        }
    }
}
} // namespace NSP_DD
