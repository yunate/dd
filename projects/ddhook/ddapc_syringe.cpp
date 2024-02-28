#include "ddhook/stdafx.h"
#include "ddhook/ddsyringe.h"
#include <tlhelp32.h>
namespace NSP_DD {

ddapc_syringe::ddapc_syringe(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath)
    : m_process(process), m_dllFullPath(dllFullPath)
{
}

bool ddapc_syringe::inject_dll(bool all)
{
    if (!ddsyringe::check_param(m_process, m_dllFullPath)) {
        return false;
    }

    PVOID remoteBuff = NULL;
    s32 insertSuccessCnt = 0;
    do {
        // 将数据写入目标进程
        remoteBuff = ::VirtualAllocEx(m_process->get_handle(), NULL, 
            m_dllFullPath.size() * 2, MEM_COMMIT, PAGE_READWRITE);
        if (remoteBuff == nullptr) {
            DDLOG(WARNING, "VirtualAllocEx failure");
            break;
        }

        SIZE_T written = 0;
        if (!::WriteProcessMemory(m_process->get_handle(), remoteBuff, 
            (void*)m_dllFullPath.data(), m_dllFullPath.size() * 2, &written) ||
            written == 0) {
            DDLOG(WARNING, "WriteProcessMemory failure");
            break;
        }

        ::THREADENTRY32 te = { 0 };
        te.dwSize = sizeof(::THREADENTRY32);
        HANDLE handleSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (INVALID_HANDLE_VALUE == handleSnap) {
            DDLOG_LASTERROR();
            break;
        }
        BOOL flag = ::Thread32First(handleSnap, &te);
        while (flag) {
            if (te.th32OwnerProcessID == m_process->get_process_info().id) {
                HANDLE handleThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                if (handleThread != NULL) {
                    if (::QueueUserAPC((PAPCFUNC)LoadLibrary, handleThread, (ULONG_PTR)remoteBuff) > 0) {
                        ++insertSuccessCnt;
                    }
                    ::CloseHandle(handleThread);
                }

                if (insertSuccessCnt > 0 && !all) {
                    break;
                }
            }

            flag = ::Thread32Next(handleSnap, &te);
        }
        ::CloseHandle(handleSnap);
    } while (0);

    if (insertSuccessCnt == 0 && remoteBuff != NULL) {
        ::VirtualFreeEx(m_process->get_handle(), remoteBuff, 0, MEM_RELEASE);
    }
    return insertSuccessCnt > 0;
}

bool ddapc_syringe::uninject_dll(u32 waitTime)
{
    return ddsyringe::uninject_dll(m_process, m_dllFullPath, waitTime);
}
} // namespace NSP_DD
