#ifndef ddwin_graphics_d2d_ddd2d_render_target_h_
#define ddwin_graphics_d2d_ddd2d_render_target_h_
#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"
#include "ddbase/ddnocopyable.hpp"

#include <d2d1.h>
#include <wrl/client.h>

#include <list>
namespace NSP_DD {
using namespace Microsoft::WRL;

////////////////////////////////// ddrender_target //////////////////////////////////
class ddrender_target
{
public:
    // 任何绘制操作前需要调用 begin_draw
    // 结束后调用 end_draw
    static void begin_draw(ID2D1RenderTarget* render_target);
    static bool end_draw(ID2D1RenderTarget* render_target);

    // 从 render target 中创建一个 bitmap 的 render target
    static ComPtr<ID2D1BitmapRenderTarget> create_compatible_render_target(ID2D1RenderTarget* render_target, const ddpoint& size);
    static ComPtr<ID2D1SolidColorBrush> create_brush(ID2D1RenderTarget* render_target, const D2D1::ColorF& color, bool hight_light);
};

////////////////////////////////// ddd2d_hwnd_render_target //////////////////////////////////
class ddd2d_hwnd_render_target
{
    DDNO_COPY_MOVE(ddd2d_hwnd_render_target);
public:
    ddd2d_hwnd_render_target() = default;
    ~ddd2d_hwnd_render_target() = default;

    struct render_target_info
    {
        bool enable = true;
        std::string name;
        ComPtr<ID2D1BitmapRenderTarget> render_target;
    };

    // 初始话, 可以重复调用来重置.
    bool init(HWND hwnd);
    bool init(const ComPtr<ID2D1HwndRenderTarget>& render_target);
    ComPtr<ID2D1HwndRenderTarget> get_d2d_factory();

    // 获取一个 bitmap render target, 如果已经存在则返回
    ComPtr<ID2D1BitmapRenderTarget> get_or_create_render_target(const std::string& name);

    // 获得所有的 bitmap render target
    std::list<render_target_info>& get_bitmap_render_targets();

    // 启用或者禁用相应的 render target
    bool enable_render_target(const std::string& name, bool enable);

    // 将所有的 bitmap render targe 绘制到 hwnd render target 上面
    bool show();

protected:
    std::list<render_target_info> m_bitmap_render_target;
    ComPtr<ID2D1HwndRenderTarget> m_render_target;
};

} // namespace NSP_DD
#endif // ddwin_graphics_d2d_ddd2d_render_target_h_

