#include "ddwin/stdafx.h"
#include "ddd2d_item_fx.h"
#include "ddwin/graphics/d2d/ddd2d_render_target.h"
#include "ddwin/graphics/d2d/ddd2d_factory.h"

namespace NSP_DD {

ddd2d_item_fx* ddd2d_item_fx::create_instance(ddd2d_coordinate_system_context* csc, ddfx_func fx, D2D1::ColorF color, float line_width, float step, s32 hit_test_zone)
{
    ddd2d_item_fx* tmp = new ddd2d_item_fx();
    tmp->m_line_width = line_width;
    tmp->m_color = color;
    tmp->m_fx = fx;
    tmp->m_step = step;
    tmp->m_csc_ctx = csc;
    tmp->m_hit_test_zone = hit_test_zone;
    DDASSERT(hit_test_zone >= 0);
    return tmp;
}

void ddd2d_item_fx::draw_inner(ID2D1RenderTarget* render_target)
{
    DDASSERT(render_target != NULL);
    ComPtr<ID2D1PathGeometry> path_geometry;
    HRESULT hr = ddd2d_factory::get_d2d_factory()->CreatePathGeometry(path_geometry.ReleaseAndGetAddressOf());
    if (FAILED(hr) || path_geometry == NULL) {
        dderror_code::set_last_error(hr);
        return;
    }
    ComPtr<ID2D1GeometrySink> sink;
    hr = path_geometry->Open(sink.ReleaseAndGetAddressOf());
    if (FAILED(hr) || sink == NULL) {
        dderror_code::set_last_error(hr);
        return;
    }

    ComPtr<ID2D1SolidColorBrush> brush = ddrender_target::create_brush(render_target, m_color, m_is_selected);
    if (brush == NULL) {
        return;
    }

    float del = m_step == 0 ? 0.05f : m_step;
    float rx = m_csc_ctx->rx() + del;
    float s = (float)m_csc_ctx->lx();
    sink->BeginFigure(m_csc_ctx->point_to_pixel({ s, (*m_fx)(s - m_offset.x) - m_offset.y }), D2D1_FIGURE_BEGIN_HOLLOW);
    for (float i = s + del; i <= rx; i += del) {
        sink->AddLine(m_csc_ctx->point_to_pixel({ i, (*m_fx)(i - m_offset.x) - m_offset.y }));
    }
    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    sink->Close();
    ComPtr<ID2D1StrokeStyle> stroke_style = NULL;
    if (is_selected()) {
        stroke_style = ddd2d_factory::create_stroke_style();
    }
    render_target->DrawGeometry(
        path_geometry.Get(),
        brush.Get(),
        m_line_width * (m_is_selected ? 2.0f : 1.0f),
        stroke_style.Get());
}

bool ddd2d_item_fx::hit_test(const NSP_DD::ddpoint& src_point)
{
    auto p = m_csc_ctx->pixel_to_point({ float(src_point.x - 5), float(src_point.y - 5) });
    auto p1 = m_csc_ctx->pixel_to_point({ float(src_point.x + 5), float(src_point.y + 5) });
    D2D1_RECT_F rect{ p.x, p.y, p1.x, p1.y };
    float del = m_step == 0 ? 0.05f : m_step;
    D2D1_POINT_2F point{ rect.left, (*m_fx)(rect.left - m_offset.x) - m_offset.y };
    float min = point.y;
    float max = point.y;
    for (float i = rect.left; i <= rect.right + del; i += del) {
        point.x = i;
        point.y = (*m_fx)(i - m_offset.x) - m_offset.y;
        if (point.y > max) {
            max = point.y;
        }
        if (point.y < min) {
            min = point.y;
        }
    }

    if (min > rect.top || max < rect.bottom) {
        return false;
    }
    return true;
}

} // namespace NSP_DD

