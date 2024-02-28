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
    DDASSERT(x + w <= width);
    DDASSERT(y + h <= height);

    s32 index = 0;
    for (u32 y1 = 0; y1 < h; ++y1) {
        for (u32 x1 = 0; x1 < w; ++x1) {
            colors[index++] = at(x1 + x, y1 + y);
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
            at(x, y) = colors[y * (width - expend_w) + x];
        }
    }

    // 填充默认值
    if (!(value == ddbgra{0, 0, 0, 0})) {
        for (u32 y = 0; y < height; ++y) {
            for (u32 x = 0; x < width; ++x) {
                if (x >= width - expend_w || y >= height - expend_h) {
                    at(x, y) = value;
                }
            }
        }
    }
}

void ddbitmap::flip_h()
{
    for (u32 y = 0; y < height; ++y) {
        for (u32 x = 0; x < width / 2; ++x) {
            std::swap(at(x, y), at(width - x - 1, y));
        }
    }
}

void ddbitmap::flip_v()
{
    for (u32 y = 0; y < height / 2; ++y) {
        for (u32 x = 0; x < width; ++x) {
            std::swap(at(x, y), at(x, height - y - 1));
        }
    }
}
} // namespace NSP_DD
