#ifndef ddwin_graphics_d2d_ddd2d_item_coord_system_h_
#define ddwin_graphics_d2d_ddd2d_item_coord_system_h_
#include "ddbase/dddef.h"
#include "ddwin/graphics/d2d/ddd2d_item_general.hpp"
#include "ddwin/graphics/d2d/ddd2d_coordinate_system_context.hpp"

namespace NSP_DD {

class ddd2d_item_coord_system : public ddd2d_item_general
{
public:
    static ddd2d_item_coord_system* create_instance(ddd2d_coordinate_system_context* csc, s32 pixels_pre_grid);
    void set_pixels_pre_grid(s32 pixels_pre_grid);
    s32 get_pixels_pre_grid();

private:
    void draw_inner(ID2D1RenderTarget* render_target) override;

private:
    ddd2d_coordinate_system_context* m_csc_ctx = nullptr;

    // 每一格像素数
    s32 m_pixels_pre_grid = 20;
};

} // namespace NSP_DD
#endif // ddwin_graphics_d2d_ddd2d_item_coord_system_h_