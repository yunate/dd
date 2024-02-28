#ifndef ddbase_windows_ddprocess_h_
#define ddbase_windows_ddprocess_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/windows/ddregister.h"
#include <memory>
#include <windows.h>
namespace NSP_DD {
class ddprocess
{
    DDNO_COPY_MOVE(ddprocess);
public:
    // 进程名称枚举所有的进程id
    // processName 进程名称, 比如 notepad.exe
    static bool get_process_ids(const std::wstring& processName, std::vector<DWORD>& ids);
    static std::wstring get_process_fullpath(HANDLE hProcess);
    static bool is_process_x64(HANDLE hProcess);
    // 获得其加载了的模块的handle, 返回值不需要销毁
    static HMODULE get_moudle(DWORD processId, const std::wstring& moudleName);
public:
    ddprocess() = default;
    ~ddprocess();

    HANDLE get_handle();
    const std::wstring& get_name();
    const std::wstring& get_fullpath();
    DWORD get_id();
    DWORD get_parent_id();
    bool is_x64();
    bool init(DWORD processId);
    // 有多个重名的取第一个
    bool init(const std::wstring& processName);
    bool init(HANDLE hProcess, char dummy);

    void uninit();

private:
    HANDLE m_hProcess = nullptr;
    std::wstring m_name;
    std::wstring m_fullPath;
    DWORD m_id = 0;
    DWORD m_parent_id = 0;
    bool m_x64 = false;
};

class ddprocess_mutex
{
    DDNO_COPY_MOVE(ddprocess_mutex);
public:
    ddprocess_mutex() = default;
    ~ddprocess_mutex() = default;

    bool init(const std::wstring& name);
    bool lock(u32 waitTime = 1000);
    void unlock();

protected:
    HANDLE m_mutex = NULL;
};

class ddsub_process
{
    DDNO_COPY_MOVE(ddsub_process);
    ddsub_process() = default;
public:
    ~ddsub_process();
    // 执行单个命令,执行该命令会开启一个子进程,执行完毕后关闭,没有上下文
    static bool exec_cmd(const char* cmd, std::string& result);
    static std::unique_ptr<ddsub_process> create_instance(const std::string& exe_path, const std::string& cmd);

    bool read(std::vector<u8>& buff);
    bool read(std::string& buff);
    bool write(const std::vector<u8>& buff);
    bool write(const std::string& buff);
    bool is_running();
    bool get_exit_code(s32& exit_code);
    bool wait(u32 time_out = INFINITE);
    bool kill(s32 exit_code);

private:
    bool init(const std::string& exe_path, const std::string& cmd);

    PROCESS_INFORMATION m_pi = { 0 };
    HANDLE m_in_w = NULL;
    HANDLE m_out_r = NULL;
    s32 m_exit_code = 0;
};
} // namespace NSP_DD
#endif // ddbase_windows_ddprocess_h_
