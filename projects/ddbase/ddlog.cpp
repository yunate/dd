#include "ddbase/stdafx.h"
#include "ddbase/ddlog.hpp"
namespace NSP_DD {
DDLOG_FILTER g_ddlog_filter = nullptr;
void set_ddlog_filter(DDLOG_FILTER filter) { g_ddlog_filter = filter; }

std::string ddloginfo_default_fmt(const ddloginfo& loginfo)
{
    if (!loginfo.file.empty() && !loginfo.line.empty()) {
        return ddstr::format("[%04d-%02d-%02d %02d-%02d-%02d] [%s] [%s:%s] %s",
            loginfo.time_point.year, loginfo.time_point.mon, loginfo.time_point.day, loginfo.time_point.hour, loginfo.time_point.min, loginfo.time_point.sec,
            loginfo.level_str.c_str(),
            loginfo.file.c_str(), loginfo.line.c_str(),
            loginfo.log.c_str());
    }

    return ddstr::format("[%04d-%02d-%02d %02d-%02d-%02d] [%s] %s",
        loginfo.time_point.year, loginfo.time_point.mon, loginfo.time_point.day, loginfo.time_point.hour, loginfo.time_point.min, loginfo.time_point.sec,
        loginfo.level_str.c_str(), loginfo.log.c_str());
}

void ddlog(int level, const wchar_t* level_str, const std::wstring& log)
{
    ddlog(level, level_str, L"", L"", log);
}
void ddlog(int level, const char* level_str, const std::string& log)
{
    ddlog(level, level_str, "", "", log);
}
void ddlog(int level, const wchar_t* level_str, const wchar_t* file, const wchar_t* line, const std::wstring& log)
{
    std::string level_stra;
    std::string filea;
    std::string linea;
    std::string loga;
    ddstr::utf16_8(level_str, level_stra);
    ddstr::utf16_8(file, filea);
    ddstr::utf16_8(line, linea);
    ddstr::utf16_8(log, loga);
    ddlog(level, level_stra.c_str(), filea.c_str(), linea.c_str(), loga.c_str());
}
void ddlog(int level, const char* level_str, const char* file, const char* line, const std::string& log)
{
    ddtime ti = ddtime::now_fmt();
    ddloginfo loginfo{ (u32)level, level_str, file, line, ti, log };
    if (g_ddlog_filter != nullptr && g_ddlog_filter(loginfo)) {
        return;
    }

    std::cout << ddloginfo_default_fmt(loginfo);
}

std::wstring last_error_msgw(DWORD errorCode)
{
    HLOCAL buff = NULL;
    ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errorCode,
        0,
        (LPWSTR)&buff,
        0,
        NULL);

    std::wstring msg = (LPWSTR)buff;
    ::LocalFree(buff);
    return msg;
}

std::string last_error_msga(DWORD errorCode)
{
    HLOCAL buff = NULL;
    ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errorCode,
        0,
        (LPSTR)&buff,
        0,
        NULL);

    std::string msg = (LPSTR)buff;
    ::LocalFree(buff);
    return msg;
}

} // namespace NSP_DD
