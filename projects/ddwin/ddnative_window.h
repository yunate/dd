#ifndef ddwin_ddnative_window_h_
#define ddwin_ddnative_window_h_

#include "ddbase/ddmini_include.h"

namespace NSP_DD {
class ddnative_window;
} // namespace NSP_DD

typedef bool(*ddwin_proc_chain)(NSP_DD::ddnative_window*, HWND, UINT, WPARAM, LPARAM, LRESULT&);

template <typename NewClass>
static bool ddwin_proc_chain_helper(NSP_DD::ddnative_window* base, HWND hWnd, UINT uMsg, WPARAM w, LPARAM l, LRESULT& result)
{
    NewClass* pThis = static_cast<NewClass*>(base);
    return pThis->win_proc(hWnd, uMsg, w, l, result);
}

#define DDWIN_PROC_CHAIN __ddwin_proc_helper__
#define DDWIN_PROC_CHAIN_DEFINE(BaseClass, NewClass)                                                                    \
public:                                                                                                                 \
    NSP_DD::ddwin_proc_helper<BaseClass, NewClass> DDWIN_PROC_CHAIN = NSP_DD::ddwin_proc_helper<BaseClass, NewClass>(this);   \


namespace NSP_DD {
template <typename NewClass, typename BaseClass>
class ddwin_proc_helper
{
public:
    template <typename T>
    void set_chain(ddnative_window* native)
    {
        static_assert(std::is_base_of<BaseClass, NewClass>::value,
            "when using DDWIN_PROC_CHAIN_DEFINE(BaseClass, NewClass), NewClass must be derived from BaseClass");
        static_assert(&NewClass::win_proc != &BaseClass::win_proc,
            "when using DDWIN_PROC_CHAIN_DEFINE(BaseClass, NewClass), NewClass must has defined bool win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result); function");
        static_cast<BaseClass*>(native)->DDWIN_PROC_CHAIN.m_win_proc_chain = ddwin_proc_chain_helper<NewClass>;
    }

    template <>
    void set_chain<void>(ddnative_window*)
    {
    }

    ddwin_proc_helper(ddnative_window* native)
    {
        set_chain<BaseClass>(native);
        m_native = native;
    }
    bool operator()(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
    {
        if (m_win_proc_chain == nullptr) {
            return false;
        }

        return m_win_proc_chain(m_native, hWnd, uMsg, wParam, lParam, result);
    }

    ddwin_proc_chain m_win_proc_chain = nullptr;
    ddnative_window* m_native = nullptr;
};
} // namespace NSP_DD

namespace NSP_DD {
class ddnative_window
{
public:
    static const wchar_t* get_class_name();

public:
    ddnative_window(const std::wstring& title);
    ~ddnative_window();

    DDWIN_PROC_CHAIN_DEFINE(ddnative_window, void);
    bool win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);

    virtual bool init_window();

public:
    HWND get_window();

    ddpoint get_size();
    void set_size(const ddpoint& size);

    ddpoint get_pos();
    void set_pos(const ddpoint& pos);

    void show();
    void hide();

    std::wstring get_title();
    void set_title(const std::wstring& title);
    void set_title(const std::string& title);

    void set_icon(HICON icon);

protected:
    virtual bool register_window_class();

protected:
    HWND m_hWnd = NULL;
    std::wstring m_title;
    ddpoint m_size{ 0,0 }; // size °üº¬±êÌâÀ¸
    ddpoint m_pos{ 0,0 };
};

} // namespace NSP_DD
#endif // ddwin_ddnative_window_h_