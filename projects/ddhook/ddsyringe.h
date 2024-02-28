#ifndef ddhook_ddsyringe_h_
#define ddhook_ddsyringe_h_

#include "ddbase/windows/ddprocess.h"
#include <memory>
#include <windows.h>

namespace NSP_DD {
class ddsyringe
{
public:
    // 检查 process 和 dllFullPath 是否合法
    static bool check_param(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath);

    // 远程执行一段代码
    static bool exec_remote(HANDLE hProcess, void* enterPoint, void* buff, u32 buffSize, u32 waitTime);

    // 卸载掉注入的dll
    static bool uninject_dll(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath, u32 waitTime = 5000);
};

/** 经典注入
1.打开进程，获取进程的句柄，
2.是在内存空间开辟一段内存空间
3.是向刚刚开辟的内存中写入需要注入DLL的路径，
4.是利用GetProcessAddree()获取LoadLibrary的地址。
5.是调用远程线程，利用LoadLibrary（）去加载DLL
*/
class ddclassic_syringe
{
    DDNO_COPY_MOVE(ddclassic_syringe);
public:
    ddclassic_syringe(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath);
    ~ddclassic_syringe() = default;

    bool inject_dll(u32 waitTime = 5000);
    bool uninject_dll(u32 waitTime = 5000);

private:
    std::shared_ptr<ddprocess> m_process;
    std::wstring m_dllFullPath;
};

/** 经典注入
注入的时候将目标入口函数点的前5个字节用当前的替换掉，防止目标进程对Load Library进行了HOOK
来检查dll是否签名导致注入不了
*/
class ddclassic_syringeex
{
    DDNO_COPY_MOVE(ddclassic_syringeex);
public:
    ddclassic_syringeex(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath);
    ~ddclassic_syringeex() = default;

    bool inject_dll(u32 waitTime = 5000);
    bool uninject_dll(u32 waitTime = 5000);

private:
    std::shared_ptr<ddprocess> m_process;
    std::wstring m_dllFullPath;
};

/** apc 注入
1.打开进程，获取进程的句柄，
2.是在内存空间开辟一段内存空间
3.是向刚刚开辟的内存中写入需要注入DLL的路径
4.遍历该进程中的所有线程，向每一个线程的apc中压入 LoadLibrary
5.等待线程调用apc
@note 由于不知道线程何时会调用apc，为了提高命中率向每个线程中压入apc执行函数，这样一来，上述申请的内存就
      得不到释放，也不知道何时该卸载注入
*/
class ddapc_syringe
{
    DDNO_COPY_MOVE(ddapc_syringe);
public:
    ddapc_syringe(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath);
    ~ddapc_syringe() = default;

    bool inject_dll(bool all);
    bool uninject_dll(u32 waitTime = 5000);

private:
    std::shared_ptr<ddprocess> m_process;
    std::wstring m_dllFullPath;
};

} // namespace NSP_DD
#endif // ddhook_ddsyringe_h_