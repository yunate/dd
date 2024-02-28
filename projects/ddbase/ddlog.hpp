#ifndef ddbase_ddlog_hpp_
#define ddbase_ddlog_hpp_

#include "ddbase/dddef.h"
#include "ddbase/macro/ddmacro.hpp"
#include "ddbase/str/ddstr.h"
#include "ddbase/ddtime.h"

#include <iostream>
namespace NSP_DD {
struct ddloginfo;
} // namespace NSP_DD
typedef bool (*DDLOG_FILTER)(const NSP_DD::ddloginfo& loginfo);

namespace NSP_DD {
struct ddloginfo
{
    u32 level;
    std::string level_str;
    std::string file;
    std::string line;
    ddtime time_point;
    std::string log;
};

std::string ddloginfo_default_fmt(const ddloginfo& loginfo);
void ddlog(int level, const wchar_t* level_str, const std::wstring& log);
void ddlog(int level, const char* level_str, const std::string& log);
void ddlog(int level, const wchar_t* level_str, const wchar_t* file, const wchar_t* line, const std::wstring& log);
void ddlog(int level, const char* level_str, const char* file, const char* line, const std::string& log);
void set_ddlog_filter(DDLOG_FILTER filter);
} // namespace NSP_DD

#define DDLOG_LEVEL_DEBUG      0
#define DDLOG_LEVEL_INFO       1
#define DDLOG_LEVEL_WARNING    2
#define DDLOG_LEVEL_ERROR      3
#define DDLOG(level, str) { NSP_DD::ddlog(DDLOG_LEVEL_ ## level, #level, str); }
#define DDLOGW(level, str) { NSP_DD::ddlog(DDLOG_LEVEL_ ## level, L#level, str); }
#define DDLOG1(level, str) { NSP_DD::ddlog(DDLOG_LEVEL_ ## level, #level, __FILE__, DDTOSTRING(__LINE__), str); }
#define DDLOG1W(level, str) { NSP_DD::ddlog(DDLOG_LEVEL_ ## level, L#level, DDCAT(L, __FILE__), DDCAT(L, DDTOSTRING(__LINE__)), str); }

namespace NSP_DD {
// 获得 ERROR_CODE 的字符串描述
extern std::wstring last_error_msgw(DWORD errorCode);
extern std::string last_error_msga(DWORD errorCode);
} // namespace NSP_DD

// GetLastError 日志记录
#define DDLOG_LASTERROR() {\
    DWORD errorCode = ::GetLastError();\
    std::wstring errorMsg = ddstr::format(L"GetLastError:0x%X(%d) %s\n", errorCode, errorCode, NSP_DD::last_error_msgw(errorCode).c_str());\
    DDLOG1W(ERROR, errorMsg);\
}
#endif // ddbase_ddlog_hpp_
