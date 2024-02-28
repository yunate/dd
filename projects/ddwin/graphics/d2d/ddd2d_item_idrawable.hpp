#ifndef ddwin_graphics_d2d_ddd2d_item_idrawable_hpp_
#define ddwin_graphics_d2d_ddd2d_item_idrawable_hpp_
#include "ddbase/dddef.h"
#include "ddbase/ddmath.h"

#include <d2d1.h>

#include <list>
#include <memory>

namespace NSP_DD {

class ddd2d_item_idrawable;
class ddd2d_item_observer
{
public:
    virtual void on_item_dirty(ddd2d_item_idrawable*) {};
};

class ddd2d_item_idrawable
{
public:
    virtual ~ddd2d_item_idrawable() = default;
    virtual void draw(ID2D1RenderTarget* render_target)
    {
        if (m_dirty) {
            m_dirty = false;
            draw_inner(render_target);
        }
    }

    inline bool is_dirty() { return m_dirty; }
    virtual void dirty(bool notify = true)
    {
        m_dirty = true;
        if (notify) {
            on_item_dirty(this);
        }
    }

    virtual bool hit_test(const NSP_DD::ddpoint&)
    {
        return false;
    }

public:
    inline void add_observer(const std::weak_ptr<ddd2d_item_observer>& observer)
    {
        m_observers.push_back(observer);
    }
    inline void remove_observer(const std::weak_ptr<ddd2d_item_observer>& observer)
    {
        auto shared = observer.lock();
        m_observers.remove_if([&shared](const std::weak_ptr<ddd2d_item_observer>& r){
            auto rshared = r.lock();
            if (rshared == nullptr || rshared == shared) {
                return true;
            }

            return false;
        });
    }
    inline void on_item_dirty(ddd2d_item_idrawable* item)
    {
        for (auto it = m_observers.begin(); it != m_observers.end();) {
            auto shared = (*it).lock();
            if (shared) {
                shared->on_item_dirty(item);
                ++it;
            } else {
                it = m_observers.erase(it);
            }
        }
    }

protected:
    virtual void draw_inner(ID2D1RenderTarget* render_target) = 0;

protected:
    std::list<std::weak_ptr<ddd2d_item_observer>> m_observers;
    bool m_dirty = true;
};

} // namespace NSP_DD
#endif // ddwin_graphics_d2d_ddd2d_item_idrawable_hpp_