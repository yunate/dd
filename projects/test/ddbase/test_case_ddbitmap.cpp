#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/ddbitmap.h"

namespace NSP_DD {
DDTEST(test_case_ddbitmap_resize, 1)
{
    ddbitmap bitmap;
    bitmap.resize(100, 100);
    if (bitmap.width != 100) {
        DDASSERT(false);
    }
    if (bitmap.height != 100) {
        DDASSERT(false);
    }
    if (bitmap.colors.size() != 100) {
        DDASSERT(false);
    }
}

DDTEST(test_case_ddbitmap_clip, 1)
{
    ddbitmap bitmap;
    bitmap.resize(8, 8);
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            bitmap.at(x, y) = {0,0,0,0};
        }
    }

    for (u32 y = bitmap.height / 4; y < bitmap.height - bitmap.height/ 4; ++y) {
        for (u32 x = bitmap.width / 4; x < bitmap.width - bitmap.width / 4; ++x) {
            bitmap.at(x, y) = { 1,1,1,1 };
        }
    }

    bitmap.clip(bitmap.width / 4, bitmap.height / 4, bitmap.width / 2, bitmap.height / 2);
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            if (!(bitmap.at(x, y) == ddbgra{ 1,1,1,1 })) {
                DDASSERT(false);
            }
        }
    }
}

DDTEST(test_case_ddbitmap_expend, 1)
{
    ddbitmap bitmap;
    bitmap.resize(4, 4);
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            bitmap.at(x, y) = { 2,2,2,2 };
        }
    }

    bitmap.expend(bitmap.width, bitmap.height, { 1,1,1,1 });
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            if (x < bitmap.width / 2 && y < bitmap.height / 2) {
                if (!(bitmap.at(x, y) == ddbgra{ 2,2,2,2 })) {
                    DDASSERT(false);
                }
            } else {
                if (!(bitmap.at(x, y) == ddbgra{ 1,1,1,1 })) {
                    DDASSERT(false);
                }
            }
        }
    }
}

DDTEST(test_case_ddbitmap_flip_h, 1)
{
    ddbitmap bitmap;
    bitmap.resize(4, 4);
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            if (x < bitmap.width / 2) {
                bitmap.at(x, y) = { 2,2,2,2 };
            } else {
                bitmap.at(x, y) = { 1,1,1,1 };
            }
        }
    }

    bitmap.flip_h();
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            if (x < bitmap.width / 2) {
                if (!(bitmap.at(x, y) == ddbgra{ 1,1,1,1 })) {
                    DDASSERT(false);
                }
            } else {
                if (!(bitmap.at(x, y) == ddbgra{ 2,2,2,2 })) {
                    DDASSERT(false);
                }
            }
        }
    }
}

DDTEST(test_case_ddbitmap_flip_v, 1)
{
    ddbitmap bitmap;
    bitmap.resize(4, 4);
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            if (y < bitmap.height / 2) {
                bitmap.at(x, y) = { 2,2,2,2 };
            } else {
                bitmap.at(x, y) = { 1,1,1,1 };
            }
        }
    }

    bitmap.flip_v();
    for (u32 y = 0; y < bitmap.height; ++y) {
        for (u32 x = 0; x < bitmap.width; ++x) {
            if (y < bitmap.height / 2) {
                if (!(bitmap.at(x, y) == ddbgra{ 1,1,1,1 })) {
                    DDASSERT(false);
                }
            } else {
                if (!(bitmap.at(x, y) == ddbgra{ 2,2,2,2 })) {
                    DDASSERT(false);
                }
            }
        }
    }
}
} // namespace NSP_DD
