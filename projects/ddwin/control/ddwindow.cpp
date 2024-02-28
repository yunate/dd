#include "ddwin/stdafx.h"
#include "ddwin/control/ddwindow.h"
#include "ddwin/style/ddstyle.h"
#include "ddwin/style/ddlayout.h"
#include "ddbase/ddassert.h"
namespace NSP_DD {

ddwindow::ddwindow()
{
}

bool ddwindow::win_proc(HWND, UINT, WPARAM, LPARAM)
{
    return false;
}

bool ddwindow::is_msg_transparent_at_point(const ddpoint& pt) const
{
    (pt);
    return false;
}

bool ddwindow::is_contain_point(const ddpoint& pt) const
{
    return m_rect.is_contian_point(pt);
}

sp_ddwindow ddwindow::child_from_point(const ddpoint& pt, bool include_msg_transparent/* = false */)
{
    if (!is_contain_point(pt)) {
        return nullptr;
    }

    for (auto it = m_childs.rbegin(); it != m_childs.rend(); ++it) {
        const sp_ddwindow& child = (*it);
        const sp_ddwindow& target = child->child_from_point(pt - child->m_rect.pos(), include_msg_transparent);
        if (target != nullptr) {
            return target;
        }
    }

    // 该位置不在任何子窗口下, 说明指向自身
    if (!include_msg_transparent && !is_msg_transparent_at_point(pt)) {
        return nullptr;
    }

    return shared_from_this();
}

void ddwindow::insert_child(const sp_ddwindow& child)
{
    for (auto it = m_childs.rbegin(); it != m_childs.rend(); ++it) {
        if ((*it)->m_top_level) {
            continue;
        }
        m_childs.insert(it.base(), child);
    }
}
} // namespace NSP_DD
