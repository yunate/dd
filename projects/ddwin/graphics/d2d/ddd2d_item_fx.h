#ifndef ddwin_graphics_d2d_ddd2d_item_fx_h_
#define ddwin_graphics_d2d_ddd2d_item_fx_h_
#include "ddbase/dddef.h"
#include "ddwin/graphics/d2d/ddd2d_item_general.hpp"
#include "ddwin/graphics/d2d/ddd2d_coordinate_system_context.hpp"

namespace NSP_DD {

typedef float(*ddfx_func)(float);
class ddd2d_item_fx : public ddd2d_item_general
{
public:
    static ddd2d_item_fx* create_instance(ddd2d_coordinate_system_context* csc, ddfx_func fx, D2D1::ColorF color, float line_width, float step = 0.05f, s32 hit_test_zone = 5);
    virtual bool hit_test(const NSP_DD::ddpoint& point) override;

private:
    void draw_inner(ID2D1RenderTarget* render_target) override;

private:
    ddd2d_coordinate_system_context* m_csc_ctx = nullptr;
    ddfx_func m_fx = nullptr;
    float m_step = 0.05f;
    float m_line_width = 1.0f;
    D2D1::ColorF m_color = D2D1::ColorF(D2D1::ColorF::Black);
    // 点击测试区域,以点击点上下左右5像素的正方形
    s32 m_hit_test_zone = 5;
};

} // namespace NSP_DD
#endif // ddwin_graphics_d2d_ddd2d_item_fx_h_