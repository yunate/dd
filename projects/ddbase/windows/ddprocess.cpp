#include "ddbase/stdafx.h"
#include "ddbase/windows/ddprocess.h"
#include "ddbase/ddassert.h"
#include "ddbase/str/ddstr.h"
#include "ddbase/dderror_code.h"
#include "ddbase/windows/ddsecurity.h"
#include "ddbase/ddlog.hpp"
#include "ddbase/str/ddstr.h"
#include <tlhelp32.h>
#include <winternl.h>
#include <windows.h>
#pragma comment(lib, "ntdll.lib")

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

void ddprocess::enum_process(const std::function<bool(const ddprocess_info&)>& pred)
{
    if (pred == nullptr) {
        return;
    }
    do {
        HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == hSnapshot) {
            DDLOG_LASTERROR();
            break;
        }

        ::PROCESSENTRY32 pe = { sizeof(pe) };
        BOOL flag = ::Process32FirstW(hSnapshot, &pe);
        while (flag) {
            ddprocess_info info;
            do {
                info.name = pe.szExeFile;
                info.id = pe.th32ProcessID;
                info.parent_id = pe.th32ParentProcessID;

                if (info.id == 0) {
                    break;
                }

                auto hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, info.id);
                if (hProcess == NULL) {
                    break;
                }

                info.x64 = is_process_x64(hProcess);
                info.fullPath = get_process_fullpath(hProcess);
                (void)::CloseHandle(hProcess);
            } while (0);

            if (!pred(info)) {
                break;
            }

            flag = ::Process32NextW(hSnapshot, &pe);
        }
        ::CloseHandle(hSnapshot);
    } while (0);
}

#pragma pack(push, 1)
typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
    PVOID       Object;
    ULONG_PTR   UniqueProcessId;
    ULONG_PTR   HandleValue;
    ULONG       GrantedAccess;
    USHORT      CreatorBackTraceIndex;
    USHORT      ObjectTypeIndex;
    ULONG       HandleAttributes;
    ULONG       Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX {
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, * PSYSTEM_HANDLE_INFORMATION_EX;

typedef struct _OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING TypeName;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG TotalPagedPoolUsage;
    ULONG TotalNonPagedPoolUsage;
    ULONG TotalNamePoolUsage;
    ULONG TotalHandleTableUsage;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG HighWaterPagedPoolUsage;
    ULONG HighWaterNonPagedPoolUsage;
    ULONG HighWaterNamePoolUsage;
    ULONG HighWaterHandleTableUsage;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    UCHAR TypeIndex;
    CHAR ReservedByte;
    ULONG PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION
{
    UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, * POBJECT_NAME_INFORMATION;
#pragma pack(pop)

struct ddrun_in_thread_with_timeout_ctx
{
    bool init()
    {
        thread_id = ::GetCurrentThreadId();
        event = ::CreateEventW(NULL, false, false, ddstr::format(L"ddrun_in_thread_with_timeout_event_%d", thread_id).c_str());
        if (event == NULL) {
            return false;
        }
        event1 = ::CreateEventW(NULL, true, false, ddstr::format(L"ddrun_in_thread_with_timeout_callback_end_event_%d", thread_id).c_str());
        if (event1 == NULL) {
            return false;
        }
        return true;
    }

    ~ddrun_in_thread_with_timeout_ctx()
    {
        end = true;
        (void)::SetEvent(event);
       (void)::WaitForSingleObject(event1, 1000);

        if (event != NULL) {
            ::CloseHandle(event);
        }
        if (event1 != NULL) {
            ::CloseHandle(event1);
        }
        if (thread != NULL) {
            ::CloseHandle(thread);
        }
    }

    DWORD thread_id = 0;
    HANDLE event = NULL;
    HANDLE event1 = NULL;
    bool end = false;
    HANDLE thread = NULL;
    std::function<void()> callback = nullptr;
};

static DWORD WINAPI run_in_thread_with_timeout_proc(LPVOID lpParam)
{
    ddrun_in_thread_with_timeout_ctx* ctx = (ddrun_in_thread_with_timeout_ctx*)lpParam;
    // 线程已经创建完毕
    (void)::SetEvent(ctx->event1);
    while (true) {
        if (::WaitForSingleObject(ctx->event, 1000) != WAIT_OBJECT_0) {
            continue;
        }

        if (ctx->end) {
            (void)::SetEvent(ctx->event1);
            break;
        }

        (ctx->callback)();
        (void)::SetEvent(ctx->event1);
    }
    return 0;
}

static bool run_in_thread_with_timeout(ddrun_in_thread_with_timeout_ctx* ctx, const std::function<void()>& callback, u32 timeout)
{
    bool result = false;
    do
    {
        if (ctx->thread == NULL) {
            DWORD thread_id = 0;
            ctx->thread = ::CreateThread(
                nullptr,
                0,
                run_in_thread_with_timeout_proc,
                (void*)(ctx),
                0,
                &thread_id
            );
            ::WaitForSingleObject(ctx->event1, INFINITE);
            ::ResetEvent(ctx->event1);
        }

        if (ctx->thread == NULL) {
            break;
        }

        ctx->callback = callback;
        (void)::SetEvent(ctx->event);
        DWORD waitResult = ::WaitForSingleObject(ctx->event1, DWORD(timeout));
        if (waitResult != WAIT_OBJECT_0) {
#pragma warning(push)
#pragma warning(disable:6258)
            ::TerminateThread(ctx->thread, 1);
#pragma warning(pop)
            ::WaitForSingleObject(ctx->thread, INFINITE);
            ctx->thread = NULL;
            ::ResetEvent(ctx->event1);
            break;
        }

        ::ResetEvent(ctx->event1);
        result = true;
    } while (false);

    return result;
}

static bool get_handles_info(
    const PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX raw_info,
    ddhandle_info& info,
    HANDLE current_process,
    ddrun_in_thread_with_timeout_ctx* ctx
)
{
    bool result = false;
    HANDLE target_process = NULL;
    HANDLE dup_handle = NULL;
    do {
        info.process_id = (DWORD)(raw_info->UniqueProcessId & 0xFFFFFFFF);
        target_process = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, info.process_id);
        if (target_process == NULL) {
            break;
        }

        if (!::DuplicateHandle(target_process, (HANDLE)raw_info->HandleValue, current_process, &dup_handle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            break;
        }

        // get type name
        u8 type_buf[512];
        ULONG return_len = 0;
        if (::NtQueryObject(dup_handle, ObjectTypeInformation, type_buf, sizeof(type_buf), &return_len) == 0) {
            auto* type_info = (POBJECT_TYPE_INFORMATION)(type_buf);
            if (type_info->TypeName.Buffer != NULL && type_info->TypeName.Length > 0) {
                info.type_name.assign(type_info->TypeName.Buffer, type_info->TypeName.Length / sizeof(WCHAR));
            }
        }

        if (info.type_name.empty()) {
            break;
        }

        // get base object name
        std::function<void()> get_base_object_name_proc = [&info, dup_handle]() {
            ULONG return_len = 0;
            u8 name_buf[2048];
            if (::NtQueryObject(dup_handle, (OBJECT_INFORMATION_CLASS)1 /* ObjectNameInformation */, name_buf, sizeof(name_buf), &return_len) == 0) {
                auto* name_info = (POBJECT_NAME_INFORMATION)(name_buf);
                if (name_info->Name.Length > 0 && name_info->Name.Buffer != NULL) {
                    info.base_object_name.assign(name_info->Name.Buffer, name_info->Name.Length / sizeof(WCHAR));
                }
            }
        };

        if ((raw_info->GrantedAccess & 0x00100000L /* SYNCHRONIZE  */) != 0) {
            // 如果句柄的GrantedAccess是SYNCHRONIZE的, NtQueryObject(OBJECT_INFORMATION_CLASS) 可能会卡住,
            // 把它放到线程里面执行并等待1ms, 如果依旧没有返回则认为是获取失败
            if (!run_in_thread_with_timeout(ctx, get_base_object_name_proc, 10)) {
                break;
            }
        } else {
            get_base_object_name_proc();
        }

        if (info.base_object_name.empty()) {
            break;
        }

        result = true;
    } while (false);

    if (target_process != NULL) {
        ::CloseHandle(target_process);
    }

    if (dup_handle != NULL) {
        ::CloseHandle(dup_handle);
    }
    return result;
}

static bool query_system_handle_info(std::vector<u8>& buffer)
{
    ULONG buffer_size = 0x10000;
    NTSTATUS status = 0;
    for (s32 i = 0; i < 10; ++i) {
        buffer.resize(buffer_size);
        status = ::NtQuerySystemInformation(
            (SYSTEM_INFORMATION_CLASS)64 /* SystemExtendedHandleInformation */,
            buffer.data(),
            buffer_size,
            &buffer_size
        );

        if (status != (NTSTATUS)0xC0000004L /* STATUS_INFO_LENGTH_MISMATCH */) {
            break;
        }
    }

    if (status < 0) {
        buffer_size = 0x10000;
        while (true) {
            buffer.resize(buffer_size);
            status = ::NtQuerySystemInformation(
                (SYSTEM_INFORMATION_CLASS)64 /* SystemExtendedHandleInformation */,
                buffer.data(),
                buffer_size,
                NULL
            );
            if (status != (NTSTATUS)0xC0000004L /* STATUS_INFO_LENGTH_MISMATCH */) {
                break;
            }

            buffer_size *= 2;
            if (buffer_size > 256 * 1024 * 1024) {
                break;
            }
        }
    }

    if (status < 0) {
        return false;
    }

    return true;
}

static const std::wstring nt_path_to_dos_path(const std::wstring& nt_path)
{
    static std::map<std::wstring, std::wstring> map;
    if (map.empty()) {
        DWORD len = ::GetLogicalDriveStringsW(0, NULL);
        std::vector<wchar_t> buff(len);
        (void)::GetLogicalDriveStringsW(len, (LPWSTR)buff.data());

        for (auto c : buff) {
            if ((c > 'A' && c < 'Z') || (c > 'a' && c < 'z')) {
                wchar_t device_name[3]{ c, L':', 0 };
                std::vector<wchar_t> buff1(1024);
                if (::QueryDosDeviceW(device_name, buff1.data(), (DWORD)buff1.size()) != 0) {
                    map[device_name] = buff1.data();
                }
            }
        }
    }

    for (auto& [key, value] : map) {
        if (::_wcsnicmp(nt_path.c_str(), value.c_str(), value.size()) == 0) {
            return key + nt_path.substr(value.size());
        }
    }

    return nt_path;
}

void ddprocess::enum_all_handles(
    const std::function<void(const ddhandle_info&)>& pred,
    s32 thread_count /* = 32 */
)
{
    if (thread_count == 0) {
        thread_count = 32;
    }

    if (pred == nullptr) {
        return;
    }

    std::vector<u8> buffer;
    if (!query_system_handle_info(buffer)) {
        return;
    }

    auto* raw_handles = (PSYSTEM_HANDLE_INFORMATION_EX)buffer.data();

    std::mutex mutex;
    std::vector<std::thread*> threads;
    auto current_process = GetCurrentProcess();
    s32 all_count = (s32)(raw_handles->NumberOfHandles);
    s32 splited_count = all_count / thread_count;
    for (s32 i = 0; i < thread_count; ++i) {
        auto* splited_raw_handles = &(raw_handles->Handles[i * (splited_count)]);
        s32 count = splited_count;
        if (i == thread_count - 1) {
            // 最后一个线程要包括所有
            count = all_count - (i * splited_count);
        }

        threads.push_back(new std::thread([&mutex, &pred, splited_raw_handles, count, current_process]() {
            ddrun_in_thread_with_timeout_ctx ctx;
            if (!ctx.init()) {
                return;
            }

            for (s32 j = 0; j < count; ++j) {
                ddhandle_info info;
                if (get_handles_info(&splited_raw_handles[j], info, current_process, &ctx)) {
                    info.base_object_name = nt_path_to_dos_path(info.base_object_name);
                    mutex.lock();
                    pred(info);
                    mutex.unlock();
                }
            }
        }));
    }

    for (s32 i = 0; i < thread_count; ++i) {
        threads[i]->join();
        delete threads[i];
    }
}

ddprocess::~ddprocess()
{
    uninit();
}

bool ddprocess::init(DWORD processId)
{
    uninit();
    do {
        m_info.id = processId;
        m_hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_info.id);
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
            if (m_info.id == pe.th32ProcessID) {
                m_info.name = pe.szExeFile;
                m_info.parent_id = pe.th32ParentProcessID;
                break;
            }
            flag = ::Process32NextW(hSnapshot, &pe);
        }

        ::CloseHandle(hSnapshot);
        if (m_info.name.empty()) {
            DDLOG(WARNING, ddstr::format("can not find process id:%d", m_info.id));
            break;
        }
        // 获取全路径
        m_info.fullPath = get_process_fullpath(m_hProcess);
        if (m_info.fullPath.empty()) {
            break;
        }

        // 是否是64位
        m_info.x64 = is_process_x64(m_hProcess);
        return true;
    } while (0);

    uninit();
    return false;
}

bool ddprocess::init(const std::wstring& name)
{
    uninit();
    do {
        m_info.name = name;

        // 获取 parent id 和 id
        HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == hSnapshot) {
            DDLOG_LASTERROR();
            break;
        }
        ::PROCESSENTRY32 pe = { sizeof(pe) };
        BOOL flag = ::Process32FirstW(hSnapshot, &pe);
        while (flag) {
            std::wstring left = m_info.name;
            std::wstring right = pe.szExeFile;
            ddstr::to_lower(left);
            ddstr::to_lower(right);
            if (left == right) {
                m_info.id = pe.th32ProcessID;
                m_info.parent_id = pe.th32ParentProcessID;
                break;
            }
            flag = ::Process32NextW(hSnapshot, &pe);
        }
        ::CloseHandle(hSnapshot);
        if (m_info.id == 0) {
            DDLOGW(WARNING, ddstr::format(L"can not find process:%s", m_info.name.c_str()));
            break;
        }

        m_hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_info.id);
        if (m_hProcess == NULL) {
            DDLOG_LASTERROR();
            break;
        }

        // 获取全路径
        m_info.fullPath = get_process_fullpath(m_hProcess);
        if (m_info.fullPath.empty()) {
            break;
        }

        // 是否是64位
        m_info.x64 = is_process_x64(m_hProcess);
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

        m_info.id = ::GetProcessId(hProcess);
        if (m_info.id == 0) {
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
            if (m_info.id == pe.th32ProcessID) {
                m_info.name = pe.szExeFile;
                m_info.parent_id = pe.th32ParentProcessID;
                break;
            }
            flag = ::Process32NextW(hSnapshot, &pe);
        }

        ::CloseHandle(hSnapshot);
        if (m_info.name.empty()) {
            DDLOG(WARNING, ddstr::format("can not find process:%x", m_hProcess));
            break;
        }

        // 获取全路径
        m_info.fullPath = get_process_fullpath(m_hProcess);
        if (m_info.fullPath.empty()) {
            break;
        }

        // 是否是64位
        m_info.x64 = is_process_x64(m_hProcess);
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
    m_info = ddprocess_info();
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