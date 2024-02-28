#include "ddwin/stdafx.h"
#include "ddwin/graphics/d2d/ddd2d_item_coord_system.h"
#include "ddwin/graphics/d2d/ddd2d_render_target.h"
#include "ddwin/graphics/d2d/ddd2d_factory.h"
#include "ddbase/dderror_code.h"

#include <d2d1.h>
#include <wrl/client.h>
using namespace Microsoft::WRL;

namespace NSP_DD {

ddd2d_item_coord_system* ddd2d_item_coord_system::create_instance(ddd2d_coordinate_system_context* csc, s32 pixels_pre_grid)
{
    DDASSERT(csc != nullptr);
    ddd2d_item_coord_system* inst = new(std::nothrow) ddd2d_item_coord_system();
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    inst->m_pixels_pre_grid = pixels_pre_grid;
    inst->m_csc_ctx = csc;
    return inst;
}

void ddd2d_item_coord_system::set_pixels_pre_grid(s32 pixels_pre_grid)
{
    DDASSERT(pixels_pre_grid > 0);
    m_pixels_pre_grid = pixels_pre_grid;
}

s32 ddd2d_item_coord_system::get_pixels_pre_grid()
{
    return m_pixels_pre_grid;
}

void ddd2d_item_coord_system::draw_inner(ID2D1RenderTarget* render_target)
{
    DDASSERT(render_target != NULL);

    ComPtr<ID2D1SolidColorBrush> light_gray_brush = ddrender_target::create_brush(render_target, D2D1::ColorF(D2D1::ColorF::LightGray), false);
    if (light_gray_brush == NULL) {
        return;
    }

    ComPtr<ID2D1SolidColorBrush> gray_brush = ddrender_target::create_brush(render_target, D2D1::ColorF(D2D1::ColorF::Gray), false);
    if (gray_brush == NULL) {
        return;
    }

    ComPtr<ID2D1SolidColorBrush> black_brush = ddrender_target::create_brush(render_target, D2D1::ColorF(D2D1::ColorF::Black), false);
    if (black_brush == NULL) {
        return;
    }

    ComPtr<IDWriteTextFormat> text_format = ddd2d_factory::CreateTextFormat(L"Arial", 12.0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
    if (text_format == NULL) {
        return;
    }

    // CSC
    render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));
    D2D1_SIZE_F target_size = render_target->GetSize();
    ddpoint origin = m_csc_ctx->get_origin();
    s32 pixels_per_grid = m_pixels_pre_grid;
    s32 pixels_per_unit = m_csc_ctx->get_pixels_per_unit();

    // 原点
    float origin_size = 3.0f;
    render_target->FillEllipse({ {(FLOAT)origin.x, (FLOAT)origin.y}, origin_size, origin_size }, black_brush.Get());

    // 横网格
    for (float i = (FLOAT)(origin.y % pixels_per_grid); i < target_size.height; i += pixels_per_grid) {
        render_target->DrawLine(
            D2D1::Point2F(0.0f, i),
            D2D1::Point2F(target_size.width, i),
            light_gray_brush.Get(),
            1.0f
        );
    }

    // 纵网格
    for (float i = float(origin.x % pixels_per_grid); i < target_size.width; i += pixels_per_grid) {
        render_target->DrawLine(
            D2D1::Point2F(i, 0.0f),
            D2D1::Point2F(i, target_size.height),
            light_gray_brush.Get(),
            1.0f
        );
    }

    // 横坐标轴
    render_target->DrawLine(
        D2D1::Point2F(0, float(origin.y)),
        D2D1::Point2F(target_size.width, float(origin.y)),
        black_brush.Get(),
        1.0f
    );

    // 横坐标刻度线/横坐标
    int xbegin = -1 * (origin.x / pixels_per_unit);
    for (float i = float(origin.x % pixels_per_unit); i < target_size.width; i += pixels_per_unit) {

        render_target->DrawLine(
            D2D1::Point2F(i, (float)origin.y - origin_size),
            D2D1::Point2F(i, (float)origin.y + origin_size),
            gray_brush.Get(),
            1.0f
        );

        if (xbegin != 0) {
            std::wstring str = ddstr::format(L"%d", xbegin);
            render_target->DrawTextW(str.c_str(), (UINT32)str.length(), text_format.Get(),
                D2D1::RectF(i - 12, origin.y + origin_size + 1, i + 12, origin.y + origin_size + 21), gray_brush.Get());
        }
        ++xbegin;
    }

    // 纵坐标轴
    render_target->DrawLine(
        D2D1::Point2F(float(origin.x), 0),
        D2D1::Point2F(float(origin.x), target_size.height),
        black_brush.Get(),
        1.0f
    );

    // 纵坐标刻度线/纵坐标
    int ybegin = (origin.y / pixels_per_unit);
    for (float i = (FLOAT)(origin.y % pixels_per_unit); i < target_size.height; i += pixels_per_unit) {
        render_target->DrawLine(
            D2D1::Point2F(origin.x - origin_size, i),
            D2D1::Point2F(origin.x + origin_size, i),
            gray_brush.Get(),
            1.0f
        );

        if (ybegin != 0) {
            std::wstring str = ddstr::format(L"%d", ybegin);
            render_target->DrawTextW(str.c_str(), (UINT32)str.length(), text_format.Get(),
                D2D1::RectF((float)origin.x - 21, i - 6, (float)origin.x - 1, i + 6), gray_brush.Get());
        }
        --ybegin;
    }
}

} // namespace NSP_DD
