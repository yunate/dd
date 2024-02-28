#ifndef ddwin_graphics_d2d_ddd2d_factory_h_
#define ddwin_graphics_d2d_ddd2d_factory_h_
#include "ddbase/dddef.h"
#include "ddbase/dderror_code.h"

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl/client.h>

namespace NSP_DD {
using namespace Microsoft::WRL;

extern bool init_factory();
extern void uninit_factory();
extern bool init_dwrite_factory();
extern void uninit_dwrite_factory();
extern bool init_wic_bitmap_factory();
extern void uninit_wic_bitmap_factory();

class ddd2d_factory
{
public:
    inline static bool init()
    {
        if (!init_factory()) {
            return false;
        }

        if (!init_dwrite_factory()) {
            return false;
        }

        if (!init_wic_bitmap_factory()) {
            return false;
        }

        return true;
    }

    inline static void uninit()
    {
        uninit_factory();
        uninit_dwrite_factory();
        uninit_wic_bitmap_factory();
    }

public:
    // ID2D1Factory
    static ComPtr<ID2D1Factory> get_d2d_factory();
    static ComPtr<ID2D1HwndRenderTarget> create_render_target_from_hwnd(HWND hwnd);
    static ComPtr<ID2D1StrokeStyle> create_stroke_style();

public:
    // IDWriteFactory
    static ComPtr<IDWriteFactory> get_dwrite_factory();
    static ComPtr<IDWriteTextFormat> CreateTextFormat(const std::wstring& fontFamilyName, FLOAT fontSize, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle);

public:
    // IWICImagingFactory
    static ComPtr<IWICImagingFactory> get_wic_image_factory();
    static ComPtr<ID2D1Bitmap> CreateBitmapSourceFromFile(ComPtr<ID2D1RenderTarget> renderTarget, const std::wstring& url);
    static ComPtr<ID2D1Bitmap> CreateBitmapSourceFromResource(ComPtr<ID2D1RenderTarget> renderTarget, int id);
};

} // namespace NSP_DD
#endif // ddwin_graphics_d2d_ddd2d_factory_h_

