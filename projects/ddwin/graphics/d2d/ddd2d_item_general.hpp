#ifndef ddwin_graphics_d2d_ddd2d_item_general_hpp_
#define ddwin_graphics_d2d_ddd2d_item_general_hpp_
#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"
#include "ddwin/graphics/d2d/ddd2d_item_idrawable.hpp"

namespace NSP_DD {

class ddd2d_item_general : public ddd2d_item_idrawable
{
public:
    inline D2D1_POINT_2F get_offset() { return m_offset; }
    virtual void set_offset(const D2D1_POINT_2F& offset)
    {
        m_offset = offset;
    }

    inline bool is_selected() { return m_is_selected; }
    virtual void select(bool is_selected)
    {
        m_is_selected = is_selected;
        dirty(true);
    }
protected:
    D2D1_POINT_2F m_offset{ 0, 0 };
    bool m_is_selected = false;
};

} // namespace NSP_DD
#endif // ddwin_graphics_d2d_ddd2d_item_general_hpp_