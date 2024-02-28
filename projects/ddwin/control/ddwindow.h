
#ifndef ddwin_control_ddwindow_h_
#define ddwin_control_ddwindow_h_

#include "ddwin/control/ddtrue_window.h"
#include "ddbase/ddmath.h"
#include <memory>
#include <list>

namespace NSP_DD {
class ddwindow;
using sp_ddwindow = std::shared_ptr<ddwindow>;
using wk_ddwindow = std::weak_ptr<ddwindow>;

class ddwindow : public std::enable_shared_from_this<ddwindow>
{

public:
    ddwindow();
    virtual ~ddwindow() = default;
    virtual bool win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool is_msg_transparent_at_point(const ddpoint& pt) const;
    bool is_contain_point(const ddpoint& pt) const;
    sp_ddwindow child_from_point(const ddpoint& pt, bool include_msg_transparent = false);
    void insert_child(const sp_ddwindow& child);
protected:
    ddrect m_rect = {0, 0, 0, 0};
    bool m_focus = false; // 是否focus, childs中只能有一个保持键盘聚焦
    bool m_capture = false; // 是否capture, childs中只能有一个保持鼠标capture
    bool m_top_level = false;
    std::list<sp_ddwindow> m_childs; // m_zorder 升序排序
    wk_ddwindow m_parent;
};

} // namespace NSP_DD
#endif // ddwin_control_ddwindow_h_
