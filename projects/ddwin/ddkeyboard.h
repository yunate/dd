
#ifndef ddwin_ddkeyboard_h_
#define ddwin_ddkeyboard_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"

#include <functional>
#include <list>
#include <queue>
#include <windows.h>
#include <bitset>
namespace NSP_DD {
struct KB_EVENT
{
    bool is_down = true;
    u8 code = 0;
    bool is_shift_down = false;
    bool is_ctrl_down = false;
    bool is_alt_down = false;
};

using KB_EVENT_CB = std::function<bool(u8)>;

class ddkeyboard
{
    DDNO_COPY_MOVE(ddkeyboard);
public:
    ddkeyboard() = default;
    ~ddkeyboard() = default;
    bool is_key_down(u8 key);
    bool is_ctrl_down();
    bool is_shift_down();
    bool is_alt_down();
    static bool is_caplock();

    bool on_msg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    bool key_down_repeat = false;
    std::list<KB_EVENT> kb_events;
    u32 kb_events_max_size = 16;
    std::bitset<256> m_keys_state;

public:
    KB_EVENT_CB ON_KEY_DOWN;
    KB_EVENT_CB ON_KEY_UP;
    KB_EVENT_CB ON_CHAR;

private:
    void update_keys_state(u8 vk, bool is_down);
    void trim_kb_events();
    void push_kb_event(const KB_EVENT& kb_event);
    bool call_cbs(const KB_EVENT_CB& cb, u8 code);
};

} // namespace NSP_DD
#endif // ddwin_ddkeyboard_h_