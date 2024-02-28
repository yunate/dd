
#include "ddwin/stdafx.h"
#include "ddwin/graphics/d2d/ddd2d_factory.h"
#include "ddbase/windows/ddmoudle_utils.h"
#include "ddbase/dderror_code.h"

namespace NSP_DD {
static ComPtr<IWICImagingFactory> s_wic_factory;

bool init_wic_bitmap_factory()
{
    if (s_wic_factory != NULL) {
        return true;
    }
    HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (LPVOID*)s_wic_factory.ReleaseAndGetAddressOf());
    if (FAILED(hr) || s_wic_factory == NULL) {
        dderror_code::set_last_error(hr);
        return false;
    }

    return true;
}

void uninit_wic_bitmap_factory()
{
    s_wic_factory.Reset();
}

ComPtr<IWICImagingFactory> ddd2d_factory::get_wic_image_factory()
{
    return s_wic_factory;
}

static HRESULT ConvertWICBitmapToD2DBitmap(ComPtr<IWICBitmapSource> src, ComPtr<ID2D1RenderTarget> renderTarget, ID2D1Bitmap** d2dBitmap)
{
    ComPtr<IWICFormatConverter> format_converter;
    HRESULT hr = s_wic_factory->CreateFormatConverter(format_converter.ReleaseAndGetAddressOf());
    if (FAILED(hr) || format_converter == NULL) {
        return hr;
    }

    hr = format_converter->Initialize(src.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        return hr;
    }

    hr = renderTarget->CreateBitmapFromWicBitmap(format_converter.Get(), nullptr, d2dBitmap);
    if (FAILED(hr) || d2dBitmap == NULL) {
        return hr;
    }
    return hr;
}

ComPtr<ID2D1Bitmap> ddd2d_factory::CreateBitmapSourceFromFile(ComPtr<ID2D1RenderTarget> renderTarget, const std::wstring& url)
{
    ComPtr<ID2D1Bitmap> bitmap;
    if (s_wic_factory == NULL) {
        return NULL;
    }

    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = s_wic_factory->CreateDecoderFromFilename(url.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.ReleaseAndGetAddressOf());
    if (FAILED(hr) || decoder == NULL) {
        dderror_code::set_last_error(hr);
        return NULL;
    }

    ComPtr<IWICBitmapFrameDecode> frame = nullptr;
    hr = decoder->GetFrame(0, frame.ReleaseAndGetAddressOf());
    if (FAILED(hr) || frame == NULL) {
        dderror_code::set_last_error(hr);
        return NULL;
    }

    hr = ConvertWICBitmapToD2DBitmap(frame, renderTarget, bitmap.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        dderror_code::set_last_error(hr);
        return NULL;
    }
    dderror_code::set_last_error(hr);
    return bitmap;
}

ComPtr<ID2D1Bitmap> ddd2d_factory::CreateBitmapSourceFromResource(ComPtr<ID2D1RenderTarget> renderTarget, int id)
{
    ComPtr<ID2D1Bitmap> bitmap;
    if (s_wic_factory == NULL) {
        return NULL;
    }

    HBITMAP hBitmap = (HBITMAP)::LoadImage(NSP_DD::ddmoudle_utils::get_moudleW(), MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    if (hBitmap == NULL) {
        return NULL;
    }

    NSP_DD::ddexec_guard guard([&]() {
        ::DeleteObject(hBitmap);
    });

    ComPtr<IWICBitmap> frame;
    HRESULT hr = s_wic_factory->CreateBitmapFromHBITMAP(hBitmap, NULL, WICBitmapUsePremultipliedAlpha, &frame);
    if (FAILED(hr) || frame == NULL) {
        dderror_code::set_last_error(hr);
        return NULL;
    }

    hr = ConvertWICBitmapToD2DBitmap(frame, renderTarget, bitmap.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        dderror_code::set_last_error(hr);
        return NULL;
    }
    return bitmap;
}
} // namespace NSP_DD
