#ifndef ddwin_graphics_d2d_ddd2d_coordinate_system_context_hpp_
#define ddwin_graphics_d2d_ddd2d_coordinate_system_context_hpp_
#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"
#include "ddbase/ddnocopyable.hpp"

#include <d2d1.h>

namespace NSP_DD {

class ddd2d_coordinate_system_context
{
public:
    inline void reset(
        const NSP_DD::ddpoint& zero_point,
        NSP_DD::s32 pixels_per_unit,
        const NSP_DD::ddpoint& total_pixels,
        const NSP_DD::ddpoint& offset,
        float scale
    )
    {
        m_zero_point_raw = zero_point;
        m_pixels_per_unit_raw = pixels_per_unit;

        set_total_pixels(total_pixels);
        set_offset(offset);
        set_scale(scale);
    }

    NSP_DD::ddpoint get_origin() const { return m_zero_point; }
    NSP_DD::s32 get_pixels_per_unit() const { return m_pixels_per_unit; }
    NSP_DD::ddpoint get_total_pixels() const { return m_total_pixels; }
    void set_total_pixels(const NSP_DD::ddpoint& pixel_size) { m_total_pixels = pixel_size; }

    // offset
    NSP_DD::ddpoint get_offset() const { return m_offset; }
    void set_offset(const NSP_DD::ddpoint& offset)
    {
        m_offset = offset;
        m_zero_point = m_offset + m_zero_point_raw;
    }

    // scale
    float get_scale() const { return m_scale; }
    void set_scale(float scale)
    {
        m_scale = scale;
        m_pixels_per_unit = (NSP_DD::s32)(m_pixels_per_unit_raw * scale);
    }

public:
    // 坐标系/像素 位置/长度相互转换
    inline D2D1_POINT_2F point_to_pixel(const D2D1_POINT_2F& point) const { return { point.x * m_pixels_per_unit + m_zero_point.x, m_zero_point.y - point.y * m_pixels_per_unit }; }
    inline D2D1_POINT_2F pixel_to_point(const D2D1_POINT_2F& pixel) const { return { (pixel.x - m_zero_point.x) / m_pixels_per_unit, (m_zero_point.y - pixel.y) / m_pixels_per_unit }; }
    inline float point_lenth_to_pixel_lenth(float lenth) { return lenth * m_pixels_per_unit; }
    inline float pixel_lenth_to_point_lenth(float lenth) { return lenth / m_pixels_per_unit; }

    // 坐标系中最小/大 x/y
    inline float lx() const { return (float)m_zero_point.x / m_pixels_per_unit * -1; }
    inline float rx() const { return (float)(m_total_pixels.x - m_zero_point.x) / m_pixels_per_unit; }
    inline float by() const { return (float)(m_zero_point.y - m_total_pixels.y) / m_pixels_per_unit; }
    inline float ty() const { return (float)m_zero_point.x / m_pixels_per_unit; }
private:
    // 该坐标系有多少像素
    NSP_DD::ddpoint m_total_pixels { 0, 0 };

    // 原始数据
    NSP_DD::ddpoint m_zero_point_raw{ 0, 0 };
    NSP_DD::s32 m_pixels_per_unit_raw = 20;

    // 偏移缩放
    NSP_DD::ddpoint m_offset{ 0, 0 };
    float m_scale = 1.0f;

    // 通过计算得到的值,但是为了效率保存一份
    NSP_DD::ddpoint m_zero_point{ 0, 0 };
    NSP_DD::s32 m_pixels_per_unit = 20;
};

} // namespace NSP_DD

#endif // ddwin_graphics_d2d_ddd2d_coordinate_system_context_hpp_