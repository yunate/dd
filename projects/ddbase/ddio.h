
#ifndef ddbase_ddio_h_
#define ddbase_ddio_h_

#include "ddbase/dddef.h"
#include <string>

#include <Windows.h>
#include <stdio.h>
namespace NSP_DD {

enum ddconsole_color
{
    gray      = FOREGROUND_INTENSITY,
    red       = FOREGROUND_INTENSITY | FOREGROUND_RED,
    green     = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    blue      = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    yellow    = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    purple    = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    cyan      = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    white     = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    black     = 0,
};

class ddcin
{
public:
    ddcin& operator>>(std::string& line);
    ddcin& operator>>(std::wstring& line);
    ddcin& scan_utf8(std::string& line);
};

class ddcout
{
public:
    ddcout() = default;
    ddcout(ddconsole_color txt_color);
    ~ddcout();

    ddcout& operator<< (const std::string& line);
    ddcout& operator<< (const std::wstring& line);
    ddcout& print_utf8(const std::string& line);

private:
    ddconsole_color m_txt_color = ddconsole_color::white;
    std::string m_line;
};

} // namespace NSP_DD
#endif // ddbase_ddio_h_
