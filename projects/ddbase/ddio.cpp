#include "ddbase/stdafx.h"
#include "ddbase/ddio.h"
#include "ddbase/str/ddstr.h"
#include "ddbase/ddlocale.h"
#include <iostream>

namespace NSP_DD {
////////////////////////////////ddcin////////////////////////////////
ddcin& ddcin::operator>>(std::string& line)
{
    line.clear();
    while (true) {
        char c = (char)::getchar();
        // ctrl + c
        if (c == 65535) {
            ::exit(0);
            break;
        }
        if (c == '\n') {
            break;
        }
        line.append(1, c);
    }
    return *this;
}

ddcin& ddcin::operator>>(std::wstring& line)
{
    line.clear();
    while (true) {
        wchar_t c = (wchar_t)::getwchar();
        // ctrl + c
        if (c == 65535) {
            ::exit(0);
            break;
        }
        if (c == L'\n') {
            break;
        }
        line.append(1, c);
    }
    return *this;
}

ddcin& ddcin::scan_utf8(std::string& line)
{
    (*this) >> line;
    line = ddstr::ansi_utf8(line);
    return *this;
}

////////////////////////////////ddcout////////////////////////////////
static std::mutex* get_ddcout_mutex()
{
    static std::mutex s_mutex;
    return &s_mutex;
}

ddcout::ddcout(ddconsole_color txt_color) : m_txt_color(txt_color) {}
ddcout::~ddcout()
{
    std::lock_guard<std::mutex> guard(*get_ddcout_mutex());
    CONSOLE_SCREEN_BUFFER_INFO info{};
    HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle != nullptr && ::GetConsoleScreenBufferInfo(handle, &info)) {
        ::SetConsoleTextAttribute(handle, (WORD)m_txt_color | black);
    } else {
        handle = nullptr;
    }
    ::printf("%s", m_line.c_str());
    if (handle != nullptr) {
        ::SetConsoleTextAttribute(handle, info.wAttributes);
    }
}

ddcout& ddcout::operator<<(const std::string& line)
{
    m_line.append(line);
    return *this;
}
ddcout& ddcout::operator<<(const std::wstring& line)
{
    m_line.append(ddstr::utf16_to(line, ddlocale::get_io_codepage()));
    return *this;
}

ddcout& ddcout::print_utf8(const std::string& line)
{
    return (*this) << ddstr::utf8_ansi(line);
}
} // namespace NSP_DD
