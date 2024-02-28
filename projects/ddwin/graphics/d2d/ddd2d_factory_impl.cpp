
#include "ddwin/stdafx.h"
#include "ddwin/graphics/d2d/ddd2d_factory.h"
#include "ddwin/ddwindow_utils.hpp"
#include "ddbase/dderror_code.h"

namespace NSP_DD {

static ComPtr<ID2D1Factory> s_factory;
bool init_factory()
{
    if (s_factory == NULL) {
        HRESULT hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, s_factory.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            dderror_code::set_last_error(hr);
            return false;
        }
    }

    return s_factory != NULL;
}

void uninit_factory()
{
    s_factory.Reset();
}

ComPtr<ID2D1Factory> ddd2d_factory::get_d2d_factory()
{
    return s_factory;
}

ComPtr<ID2D1HwndRenderTarget> ddd2d_factory::create_render_target_from_hwnd(HWND hwnd)
{
    DDASSERT(s_factory != NULL);
    DDASSERT(hwnd != NULL);
    ComPtr<ID2D1HwndRenderTarget> target;
    RECT rect;
    NSP_DD::ddwindow_utils::get_client_rect(hwnd, rect);
    HRESULT hr = s_factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU((UINT)(rect.right - rect.left), (UINT)(rect.bottom - rect.top))),
        target.ReleaseAndGetAddressOf());
    if (FAILED(hr) || target == NULL) {
        dderror_code::set_last_error(hr);
        return NULL;
    }

    target->SetDpi(96, 96);
    return target;
}

ComPtr<ID2D1StrokeStyle> ddd2d_factory::create_stroke_style()
{
    DDASSERT(s_factory != NULL);
    ComPtr<ID2D1StrokeStyle> style;
    // Dash array for dashStyle D2D1_DASH_STYLE_CUSTOM
    HRESULT hr = s_factory->CreateStrokeStyle(
        D2D1::StrokeStyleProperties(
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_ROUND,
            D2D1_LINE_JOIN_MITER,
            10.0f,
            D2D1_DASH_STYLE_DOT,
            0.0f),
        NULL, 0,
        style.ReleaseAndGetAddressOf());
    if (FAILED(hr) || style == NULL) {
        dderror_code::set_last_error(hr);
        return NULL;
    }
    return style;
}

} // namespace NSP_DD
