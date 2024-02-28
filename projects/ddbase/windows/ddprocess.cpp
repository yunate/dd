#include "ddbase/stdafx.h"
#include "ddbase/windows/ddprocess.h"
#include "ddbase/ddassert.h"
#include "ddbase/str/ddstr.h"
#include "ddbase/dderror_code.h"
#include "ddbase/windows/ddsecurity.h"
#include "ddbase/ddlog.hpp"
#include <tlhelp32.h>

namespace NSP_DD {
bool ddprocess::get_process_ids(const std::wstring& processName, std::vector<DWORD>& ids)
{
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) {
        DDLOG_LASTERROR();
        return false;
    }
    ::PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL flag = ::Process32FirstW(hSnapshot, &pe);
    while (flag) {
        if (processName == pe.szExeFile) {
            ids.push_back(pe.th32ProcessID);
        }
        flag = ::Process32NextW(hSnapshot, &pe);
    }

    ::CloseHandle(hSnapshot);
    return true;
}

std::wstring ddprocess::get_process_fullpath(HANDLE hProcess)
{
    // 获取全路径
    WCHAR fullPath[MAX_PATH + 1];
    DWORD buffSize = ARRAYSIZE(fullPath) - 1;
    if (0 == ::QueryFullProcessImageNameW(hProcess, 0, fullPath, &buffSize)) {
        DDLOG_LASTERROR();
        return L"";
    }
    return fullPath;
}

bool ddprocess::is_process_x64(HANDLE hProcess)
{
    HMODULE hModule = ::GetModuleHandleW(L"kernel32");
    if (hModule == NULL) {
        return false;
    }
    typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)::GetProcAddress(hModule, "IsWow64Process");
    if (NULL == fnIsWow64Process) {
        return false;
    }

    BOOL bIsWow64 = FALSE;
    fnIsWow64Process(hProcess, &bIsWow64);
    return (bIsWow64 == FALSE);
}


HMODULE ddprocess::get_moudle(DWORD processId, const std::wstring& moudleName)
{
    if (moudleName.empty()) {
        return NULL;
    }
    DDASSERT(processId != 0);
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    if (INVALID_HANDLE_VALUE == hSnapshot) {
        DDLOG_LASTERROR();
        return NULL;
    }

    bool find = false;
    MODULEENTRY32 me = { 0 };
    me.dwSize = sizeof(MODULEENTRY32);
    BOOL result = ::Module32First(hSnapshot, &me);
    while (result) {
        if (moudleName == me.szModule) {
            find = true;
            break;
        }
        result = ::Module32Next(hSnapshot, &me);
    }

    ::CloseHandle(hSnapshot);
    if (!find) {
        return NULL;
    }
    return me.hModule;
}

ddprocess::~ddprocess()
{
    uninit();
}

HANDLE ddprocess::get_handle()
{
    return m_hProcess;
}

const std::wstring& ddprocess::get_name()
{
    return m_name;
}

const std::wstring& ddprocess::get_fullpath()
{
    return m_fullPath;
}

DWORD ddprocess::get_id()
{
    return m_id;
}

DWORD ddprocess::get_parent_id()
{
    return m_parent_id;
}

bool ddprocess::is_x64()
{
    return m_x64;
}

bool ddprocess::init(DWORD processId)
{
    uninit();
    do {
        m_id = processId;
        m_hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_id);
        if (m_hProcess == NULL) {
            DDLOG_LASTERROR();
            break;
        }

        // 获取 parent id 和 name
        HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == hSnapshot) {
            DDLOG_LASTERROR();
            break;
        }
        ::PROCESSENTRY32 pe = { sizeof(pe) };
        BOOL flag = ::Process32FirstW(hSnapshot, &pe);
        while (flag) {
            if (m_id == pe.th32ProcessID) {
                m_name = pe.szExeFile;
                m_parent_id = pe.th32ModuleID;
                break;
            }
            flag = ::Process32NextW(hSnapshot, &pe);
        }

        ::CloseHandle(hSnapshot);
        if (m_name.empty()) {
            DDLOG(WARNING, ddstr::format("can not find process id:%d", m_id));
            break;
        }
        // 获取全路径
        m_fullPath = get_process_fullpath(m_hProcess);
        if (m_fullPath.empty()) {
            break;
        }

        // 是否是64位
        m_x64 = is_process_x64(m_hProcess);
        return true;
    } while (0);

    uninit();
    return false;
}

bool ddprocess::init(const std::wstring& name)
{
    uninit();
    do {
        m_name = name;

        // 获取 parent id 和 id
        HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == hSnapshot) {
            DDLOG_LASTERROR();
            break;
        }
        ::PROCESSENTRY32 pe = { sizeof(pe) };
        BOOL flag = ::Process32FirstW(hSnapshot, &pe);
        while (flag) {
            std::wstring left = m_name;
            std::wstring right = pe.szExeFile;
            ddstr::to_lower(left);
            ddstr::to_lower(right);
            if (left == right) {
                m_id = pe.th32ProcessID;
                m_parent_id = pe.th32ParentProcessID;
                break;
            }
            flag = ::Process32NextW(hSnapshot, &pe);
        }
        ::CloseHandle(hSnapshot);
        if (m_id == 0) {
            DDLOGW(WARNING, ddstr::format(L"can not find process:%s", m_name.c_str()));
            break;
        }

        m_hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_id);
        if (m_hProcess == NULL) {
            DDLOG_LASTERROR();
            break;
        }

        // 获取全路径
        m_fullPath = get_process_fullpath(m_hProcess);
        if (m_fullPath.empty()) {
            break;
        }

        // 是否是64位
        m_x64 = is_process_x64(m_hProcess);
        return true;
    } while (0);

    uninit();
    return false;
}

bool ddprocess::init(HANDLE hProcess, char dummy)
{
    dummy;
    uninit();
    do {
        m_hProcess = hProcess;
        if (m_hProcess == nullptr) {
            DDLOG(WARNING, "hProcess is nullptr");
            break;
        }

        m_id = ::GetProcessId(hProcess);
        if (m_id == 0) {
            DDLOG_LASTERROR();
            break;
        }

        // 获取 parent id 和 name
        HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == hSnapshot) {
            DDLOG_LASTERROR();
            break;
        }
        ::PROCESSENTRY32 pe = { sizeof(pe) };
        BOOL flag = ::Process32FirstW(hSnapshot, &pe);
        while (flag) {
            if (m_id == pe.th32ProcessID) {
                m_name = pe.szExeFile;
                m_parent_id = pe.th32ModuleID;
                break;
            }
            flag = ::Process32NextW(hSnapshot, &pe);
        }

        ::CloseHandle(hSnapshot);
        if (m_name.empty()) {
            DDLOG(WARNING, ddstr::format("can not find process:%x", m_hProcess));
            break;
        }

        // 获取全路径
        m_fullPath = get_process_fullpath(m_hProcess);
        if (m_fullPath.empty()) {
            break;
        }

        // 是否是64位
        m_x64 = is_process_x64(m_hProcess);
        return true;
    } while (0);
    return false;
}

void ddprocess::uninit()
{
    if (m_hProcess != nullptr) {
        (void)::CloseHandle(m_hProcess);
    }
    m_hProcess = nullptr;
    m_name.clear();
    m_fullPath.clear();
    m_id = 0;
    m_parent_id = 0;
    m_x64 = false;
}

////////////////////////////////ddprocess_mutex//////////////////////////////////////////
bool ddprocess_mutex::init(const std::wstring& name)
{
    // 以低权限创建
    SECURITY_ATTRIBUTES sa {};
    sa.bInheritHandle = FALSE;
    SECURITY_DESCRIPTOR sd {};
    sa.lpSecurityDescriptor = (void*)&sd;
    if (!ddsecurity::create_low_sa(sa)) {
        return false;
    }

    m_mutex = ::CreateMutexW(&sa, false, (name + L"_MUTEX").c_str());
    return (m_mutex != NULL);
}

bool ddprocess_mutex::lock(u32 waitTime /*= 1000*/)
{
    if (m_mutex == NULL) {
        return false;
    }
    DWORD hr = ::WaitForSingleObject(m_mutex, waitTime);
    if (hr == WAIT_ABANDONED || hr == WAIT_OBJECT_0) {
        return true;
    }
    return false;
}

void ddprocess_mutex::unlock()
{
    if (m_mutex != NULL) {
        (void)::ReleaseMutex(m_mutex);
    }
}

////////////////////////////////ddsub_process//////////////////////////////////////////
ddsub_process::~ddsub_process()
{
    if (m_pi.hProcess != NULL) {
        kill(0);
        ::CloseHandle(m_pi.hProcess);
        ::CloseHandle(m_pi.hThread);
        ::CloseHandle(m_in_w);
        ::CloseHandle(m_out_r);
    }
}

bool ddsub_process::exec_cmd(const char* cmd, std::string& result)
{
    result.clear();
    char buffer[128] = { 0 };
    std::unique_ptr<FILE, decltype(&::_pclose)> pipe(::_popen(cmd, "r"), _pclose);
    if (!pipe) {
        return false;
    }

    while (::fgets(buffer, 128, pipe.get()) != nullptr) {
        result += buffer;
    }

    return true;
}

std::unique_ptr<ddsub_process> ddsub_process::create_instance(const std::string& exe_path, const std::string& cmd)
{
    std::unique_ptr<ddsub_process> instance(new(std::nothrow) ddsub_process());
    if (instance == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    if (!instance->init(exe_path, cmd)) {
        return nullptr;
    }
    return instance;
}

bool ddsub_process::read(std::vector<u8>& buff)
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }

    buff.clear();
    char read_buff[4096] = {0};
    DWORD readed = 0;
    for (;;)
    {
        if (!::ReadFile(m_out_r, read_buff, 4096, &readed, NULL)) {
            break;
        }

        size_t raw_size = buff.size();
        buff.resize(raw_size + readed);
        (void)::memcpy_s(buff.data() + raw_size, raw_size + readed, read_buff, readed);
    }
    return true;
}

bool ddsub_process::read(std::string& buff)
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }

    buff.clear();
    char read_buff[4096] = { 0 };
    DWORD readed = 0;
    for (;;)
    {
        DWORD avaiable = 0;
        if (!::PeekNamedPipe(m_out_r, NULL, 0, NULL, &avaiable, NULL) || avaiable <= 0) {
            break;
        }

        if (!::ReadFile(m_out_r, read_buff, 4096, &readed, NULL)) {
            break;
        }

        buff.append(read_buff, readed);
    }
    return true;
}

bool ddsub_process::write(const std::vector<u8>& buff)
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }

    DWORD writed = 0;
    if (!::WriteFile(m_in_w, buff.data(), (DWORD)buff.size(), &writed, NULL)) {
        return false;
    }
    return true;
}

bool ddsub_process::write(const std::string& buff)
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }

    DWORD writed = 0;
    if (!::WriteFile(m_in_w, buff.data(), (DWORD)buff.size(), &writed, NULL)) {
        return false;
    }

    if (!::FlushFileBuffers(m_in_w)) {
        return false;
    }
    return true;
}

bool ddsub_process::is_running()
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }

    if (!::GetExitCodeProcess(m_pi.hProcess, (LPDWORD) &m_exit_code)) {
        return false;
    }
    return m_exit_code == STILL_ACTIVE;
}

bool ddsub_process::get_exit_code(s32& exit_code)
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }
    exit_code = m_exit_code;
    return true;
}

bool ddsub_process::wait(u32 time_out)
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }

    if (::WaitForSingleObject(m_pi.hProcess, time_out) == WAIT_OBJECT_0) {
        return true;
    }

    return false;
}

bool ddsub_process::kill(s32 exit_code)
{
    if (m_pi.hProcess == NULL) {
        dderror_code::set_last_error(dderror_code::init_failure);
        return false;
    }
    auto code = exit_code ? static_cast<DWORD>(exit_code) : STATUS_CONTROL_C_EXIT;
    (void)::TerminateProcess(m_pi.hProcess, code);
    return wait();
}

bool ddsub_process::init(const std::string& exe_path, const std::string& cmd)
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    HANDLE in_r = NULL;
    HANDLE out_w = NULL;

    if (!::CreatePipe(&in_r, &m_in_w, &sa, 0)) {
        return false;
    }

    if (!::SetHandleInformation(m_in_w, HANDLE_FLAG_INHERIT, 0)) {
        return false;
    }

    if (!::CreatePipe(&m_out_r, &out_w, &sa, 0)) {
        return false;
    }

    if (!::SetHandleInformation(m_out_r, HANDLE_FLAG_INHERIT, 0)) {
        return false;
    }

    STARTUPINFOA si = { 0 };
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = out_w;
    si.hStdOutput = out_w;
    si.hStdInput = in_r;
    si.dwFlags |= STARTF_USESTDHANDLES;
    if (!::CreateProcessA(exe_path.empty() ? NULL : (char*)exe_path.c_str(), (char*)cmd.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &m_pi)) {
        return false;
    }

    // 关闭传给子进程的管道句柄
    ::CloseHandle(in_r);
    ::CloseHandle(out_w);
    return true;
}

} // namespace NSP_DD