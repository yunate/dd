#include "ddhook/stdafx.h"
#include "ddhook/ddsyringe.h"

namespace NSP_DD {
bool ddsyringe::check_param(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath)
{
    if (process == nullptr || process->get_process_info().id == 0) {
        DDLOG(WARNING, "m_process is invalid");
        return false;
    }

    if (!dddir::is_path_exist(dllFullPath) || dddir::is_dir(dllFullPath)) {
        DDLOGW(WARNING, ddstr::format(L"m_dllFullPath is not a valid dll path:%s", dllFullPath.c_str()));
        return false;
    }

    HANDLE hCurrentProcess = ::GetCurrentProcess();
    if (hCurrentProcess == NULL) {
        DDLOG_LASTERROR();
        return false;
    }

    if (ddprocess::is_process_x64(hCurrentProcess) != process->get_process_info().x64) {
        DDLOG1(WARNING, ddstr::format("target is %s, when current is not", process->get_process_info().x64 ? "x64" : "win32").c_str());
        return false;
    }

    return true;
}

bool ddsyringe::exec_remote(HANDLE hProcess, void* enterPoint, void* buff, u32 buffSize, u32 waitTime)
{
    DDASSERT(hProcess != NULL);
    DDASSERT(enterPoint != NULL);
    DDASSERT(buff != NULL);
    bool result = false;
    PVOID remoteBuff = NULL;
    HANDLE hRemoteThread = NULL;
    do {
        // 将数据写入目标进程
        remoteBuff = ::VirtualAllocEx(hProcess, NULL, buffSize, MEM_COMMIT, PAGE_READWRITE);
        if (remoteBuff == nullptr) {
            DDLOG(WARNING, "VirtualAllocEx failure");
            break;
        }

        SIZE_T written = 0;
        if (!::WriteProcessMemory(hProcess, remoteBuff, buff, buffSize, &written) ||
            written == 0) {
            DDLOG(WARNING, "WriteProcessMemory failure");
            break;
        }

        // 在目标进程执行
        hRemoteThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)enterPoint, remoteBuff, 0, NULL);
        if (hRemoteThread == NULL) {
            DDLOG_LASTERROR();
            return false;
        }
        ::WaitForSingleObject(hRemoteThread, (DWORD)waitTime);
        result = true;
    } while (0);


    if (remoteBuff != NULL) {
        ::VirtualFreeEx(hProcess, remoteBuff, 0, MEM_RELEASE);
    }

    if (hRemoteThread != NULL) {
        ::CloseHandle(hRemoteThread);
    }
    return result;
}

bool ddsyringe::uninject_dll(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath, u32 waitTime /*= 5000*/)
{
    if (!ddsyringe::check_param(process, dllFullPath)) {
        return false;
    }

    HANDLE hRemoteThread = NULL;
    bool result = false;
    do {
        HMODULE hModule = ddprocess::get_moudle(process->get_process_info().id, ddpath::name(dllFullPath));
        if (hModule == NULL) {
            break;
        }

        // 在目标进程执行
        hRemoteThread = ::CreateRemoteThread(process->get_handle(), NULL, 0, (LPTHREAD_START_ROUTINE)&::FreeLibrary,
            hModule, 0, NULL);
        if (hRemoteThread == NULL) {
            DDLOG_LASTERROR();
            return false;
        }
        ::WaitForSingleObject(hRemoteThread, (DWORD)waitTime);
        result = true;
    } while (0);
    if (hRemoteThread != NULL) {
        ::CloseHandle(hRemoteThread);
    }
    return result;
}
} // namespace NSP_DD
