#ifndef ddbase_windows_ddprocess_h_
#define ddbase_windows_ddprocess_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/windows/ddregister.h"
#include <functional>
#include <memory>
#include <windows.h>
namespace NSP_DD {

struct ddprocess_info
{
    std::wstring name;
    std::wstring fullPath;
    DWORD id = 0;
    DWORD parent_id = 0;
    bool x64 = false;
};

struct ddhandle_info
{
    ULONG_PTR process_id = 0;
    std::wstring type_name;
    std::wstring base_object_name;
};

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

    // 枚举进程
    // @prarm pred return false表示停止枚举
    static void enum_process(const std::function<bool(const ddprocess_info&)>& pred);

    // 枚举文件被那些进程占用了
    // 有极低的可能性crash, 这个crash无法避免, 也没有更好的解决方案, 因此建议在子进程中掉该函数
    // @param thread_count: 由于调用了NtQueryObject,
    //   使用NtQueryObject函数获得handle name时候有可能会阻塞住, 因此将它放到一个线程中执行, 如果等待了一段时间还没有返回则认为获取失败
    //   因此我们将指定thread_count数量的线程来并行
    // @param pred: 每个结果都会回调该函数, 它是线程安全的, 调用者无需加锁
    static bool enum_all_handles(
        const std::function<void(const ddhandle_info&)>& pred,
        s32 thread_count = 32
    );
    static bool enum_file_handles(
        const std::function<void(const ddhandle_info&)>& pred,
        s32 thread_count = 32
    );
public:
    ddprocess() = default;
    ~ddprocess();

    // 有多个重名的取第一个
    bool init(const std::wstring& processName);
    bool init(HANDLE hProcess, char dummy);
    bool init(DWORD processId);
    void uninit();

    const ddprocess_info& get_process_info()
    {
        return m_info;
    }

    HANDLE get_handle()
    {
        return m_hProcess;
    }

private:
    HANDLE m_hProcess = nullptr;
    ddprocess_info m_info;
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

    void ignore_output();
    bool read(std::vector<u8>& buff);
    bool read(std::string& buff);
    bool write(const std::vector<u8>& buff);
    bool write(const std::string& buff);
    bool is_running();
    bool get_exit_code(s32& exit_code);

    // before wait the subprocess to exit,
    // you should call read() function to read the subprocess's output
    // or call ignore_output() function to ignore the subprocess's output
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
