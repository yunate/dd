#include "ddbase/stdafx.h"
#include "ddbase/ddio.h"
#include "ddbase/str/ddstr.h"
#include <iostream>

namespace NSP_DD {
////////////////////////////////ddcin////////////////////////////////
ddcin& ddcin::operator>>(std::string& line)
{
    line.clear();
    while (true) {
        char c = (char)::getchar();
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
ddcout::ddcout(ddconsole_color txt_color)
{
    m_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (m_handle != nullptr && ::GetConsoleScreenBufferInfo(m_handle, &m_info)) {
        ::SetConsoleTextAttribute(m_handle, (WORD)txt_color | black);
    } else {
        m_handle = nullptr;
    }
}

ddcout::~ddcout()
{
    if (m_handle != nullptr) {
        ::SetConsoleTextAttribute(m_handle, m_info.wAttributes);
    }
}
ddcout& ddcout::operator<<(const std::string& line)
{
    ::printf("%s", line.c_str());
    return *this;
}
ddcout& ddcout::operator<<(const std::wstring& line)
{
    ::wprintf(L"%s", line.c_str());
    return *this;
}

ddcout& ddcout::print_utf8(const std::string& line)
{
    return (*this) << ddstr::utf8_ansi(line);
}
} // namespace NSP_DD
