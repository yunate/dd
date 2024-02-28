#include "ddwin/stdafx.h"
#include "ddwin/ddkeyboard.h"
namespace NSP_DD {
bool ddkeyboard::is_key_down(u8 key)
{
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    return (m_keys_state[key] == 1);
}

bool ddkeyboard::is_ctrl_down()
{
    return is_key_down(VK_CONTROL) || is_key_down(VK_LCONTROL) || is_key_down(VK_RCONTROL);
}
bool ddkeyboard::is_shift_down()
{
    return is_key_down(VK_SHIFT) || is_key_down(VK_LSHIFT) || is_key_down(VK_RSHIFT);
}
bool ddkeyboard::is_alt_down()
{
    return is_key_down(VK_MENU) || is_key_down(VK_LMENU) || is_key_down(VK_RMENU);
}

bool ddkeyboard::is_caplock()
{
    return (::GetKeyState(VK_CAPITAL) & 1);
}

//////////////////////////////////////////////////////////////////////////
bool ddkeyboard::on_msg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    hWnd;
    bool result = false;
    switch (msg)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        // 一个按键常按会触发多次 KEYDOWN, 0x40000000 表示上一个键状态.
        // 如果在发送消息之前键处于按下状态, 则值为 1; 如果键处于未按下状态，则值为 0.
        // 如果key_down_repeat为true, 表示允许多次KEYDOWN.
        if (!(lParam & 0x40000000) || key_down_repeat) {
            update_keys_state(static_cast<u8>(wParam), true);
            result = call_cbs(ON_KEY_DOWN, static_cast<u8>(wParam));
            push_kb_event(KB_EVENT{ true, static_cast<u8>(wParam) });
            trim_kb_events();
        }
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        update_keys_state(static_cast<u8>(wParam), false);
        result = call_cbs(ON_KEY_UP, static_cast<u8>(wParam));
        push_kb_event(KB_EVENT{ false, static_cast<u8>(wParam) });
        trim_kb_events();
        break;
    case WM_CHAR:
        result = call_cbs(ON_CHAR, static_cast<u8>(wParam));
        break;
    }
    return result;
}

void ddkeyboard::update_keys_state(u8 vk, bool is_down)
{
    m_keys_state[vk] = is_down ? 1 : 0;
}

void ddkeyboard::trim_kb_events()
{
    if (kb_events.size() > kb_events_max_size) {
        kb_events.pop_front();
    }
}
void ddkeyboard::push_kb_event(const KB_EVENT& kb_event)
{
    kb_events.push_back({ kb_event.is_down, kb_event.code, is_shift_down(), is_alt_down(), is_ctrl_down() });
    trim_kb_events();
}

bool ddkeyboard::call_cbs(const KB_EVENT_CB& cb, u8 code)
{
    if (cb == nullptr) {
        return false;
    }
    return cb(code);
}
} // namespace NSP_DD

