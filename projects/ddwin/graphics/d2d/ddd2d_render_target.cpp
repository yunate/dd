
#include "ddwin/stdafx.h"
#include "ddwin/graphics/d2d/ddd2d_render_target.h"
#include "ddwin/ddwindow_utils.hpp"
#include "ddwin/graphics/d2d/ddd2d_factory.h"

namespace NSP_DD {
////////////////////////////////// ddrender_target //////////////////////////////////
void ddrender_target::begin_draw(ID2D1RenderTarget* render_target)
{
    DDASSERT(render_target != NULL);
    render_target->BeginDraw();
}

bool ddrender_target::end_draw(ID2D1RenderTarget* render_target)
{
    DDASSERT(render_target != NULL);
    auto hr = render_target->EndDraw();
    if (FAILED(hr)) {
        dderror_code::set_last_error(hr);
        return false;
    }
    return true;
}

ComPtr<ID2D1BitmapRenderTarget> ddrender_target::create_compatible_render_target(ID2D1RenderTarget* render_target, const ddpoint& size)
{
    DDASSERT(render_target != NULL);
    ComPtr<ID2D1BitmapRenderTarget> target;
    auto hr = render_target->CreateCompatibleRenderTarget(D2D1_SIZE_F{ (float)size.x, (float)size.y }, target.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        dderror_code::set_last_error(hr);
        return NULL;
    }
    return target;
}

ComPtr<ID2D1SolidColorBrush> ddrender_target::create_brush(ID2D1RenderTarget* render_target, const D2D1::ColorF& color, bool hight_light)
{
    DDASSERT(render_target != NULL);
    D2D1::ColorF c = color;
    if (hight_light) {
        c = D2D1::ColorF::Black;
    }
    ComPtr<ID2D1SolidColorBrush> brush;
    HRESULT hr = render_target->CreateSolidColorBrush(c, brush.ReleaseAndGetAddressOf());
    if (FAILED(hr) || brush == NULL) {
        dderror_code::set_last_error(hr);
        return NULL;
    }
    return brush;
}

////////////////////////////////// ddd2d_hwnd_render_target //////////////////////////////////
bool ddd2d_hwnd_render_target::init(HWND hwnd)
{
    DDASSERT(hwnd != NULL);
    m_bitmap_render_target.clear();
    m_render_target = ddd2d_factory::create_render_target_from_hwnd(hwnd);
    if (m_render_target == NULL) {
        return false;
    }

    // 因为D2D的单位是dpis(比如系统DPI是120那么dpis为120/96),为了不需要放缩计算,将Dpi设置为默认值96
    // 这样D2D的单位就和像素一样了(为1)
    m_render_target->SetDpi(96, 96);
    return true;
}

bool ddd2d_hwnd_render_target::init(const ComPtr<ID2D1HwndRenderTarget>& render_target)
{
    DDASSERT(render_target != NULL);
    m_bitmap_render_target.clear();
    m_render_target = render_target;
    if (m_render_target == NULL) {
        return false;
    }

    // 因为D2D的单位是dpis(比如系统DPI是120那么dpis为120/96),为了不需要放缩计算,将Dpi设置为默认值96
    // 这样D2D的单位就和像素一样了(为1)
    m_render_target->SetDpi(96, 96);
    return true;
}

ComPtr<ID2D1HwndRenderTarget> ddd2d_hwnd_render_target::get_d2d_factory()
{
    return m_render_target;
}

ComPtr<ID2D1BitmapRenderTarget> ddd2d_hwnd_render_target::get_or_create_render_target(const std::string& name)
{
    DDASSERT(m_render_target != NULL);
    for (auto& it : m_bitmap_render_target) {
        if (it.name == name) {
            return it.render_target;
        }
    }

    ComPtr<ID2D1BitmapRenderTarget> target;
    (void)m_render_target->CreateCompatibleRenderTarget(target.ReleaseAndGetAddressOf());
    if (target == NULL) {
        return NULL;
    }

    m_bitmap_render_target.push_back({ true, name, target });
    return target.Get();
}

std::list<ddd2d_hwnd_render_target::render_target_info>& ddd2d_hwnd_render_target::get_bitmap_render_targets()
{
    return m_bitmap_render_target;
}

bool ddd2d_hwnd_render_target::enable_render_target(const std::string& name, bool enable)
{
    for (auto& it : m_bitmap_render_target) {
        if (it.name == name) {
            it.enable = enable;
            return true;
        }
    }

    return false;
}

bool ddd2d_hwnd_render_target::show()
{
    DDASSERT(m_render_target != NULL);

    ddrender_target::begin_draw(m_render_target.Get());

    for (auto& it : m_bitmap_render_target) {
        if (!it.enable) {
            continue;
        }
        ComPtr<ID2D1Bitmap> bitmap;
        HRESULT hr = it.render_target->GetBitmap(&bitmap);
        if (FAILED(hr) || bitmap == NULL) {
            return false;
        }
        D2D1_SIZE_F size = it.render_target->GetSize();
        m_render_target->DrawBitmap(
            bitmap.Get(),
            D2D1::RectF(0.0f, 0.0f, size.width, size.height),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            D2D1::RectF(0.0f, 0.0f, size.width, size.height)
        );
    }

    return ddrender_target::end_draw(m_render_target.Get());
}

} // namespace NSP_DD