#include "ddhook/stdafx.h"
#include "ddhook/ddsyringe.h"
#include "ddhook/ddsyringe.h"
#include <tlhelp32.h>
namespace NSP_DD {

ddclassic_syringe::ddclassic_syringe(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath)
    : m_process(process), m_dllFullPath(dllFullPath)
{ }

bool ddclassic_syringe::inject_dll(u32 waitTime /*= 5000*/)
{
    if (!ddsyringe::check_param(m_process, m_dllFullPath)) {
        return false;
    }

    char* enterPoint = (char*)&LoadLibraryW;
    return ddsyringe::exec_remote(m_process->get_handle(), enterPoint,
        (void*)m_dllFullPath.data(), (u32)m_dllFullPath.size() * 2, waitTime);
}

bool ddclassic_syringe::uninject_dll(u32 waitTime /*= 5000*/)
{
    return ddsyringe::uninject_dll(m_process, m_dllFullPath, waitTime);
}

ddclassic_syringeex::ddclassic_syringeex(const std::shared_ptr<ddprocess>& process, const std::wstring& dllFullPath)
    : m_process(process), m_dllFullPath(dllFullPath)
{ }

bool ddclassic_syringeex::inject_dll(u32 waitTime)
{
    if (!ddsyringe::check_param(m_process, m_dllFullPath)) {
        return false;
    }

    bool result = false;
    PVOID remoteBuff = NULL;
    HANDLE hRemoteThread = NULL;
    do {
        remoteBuff = ::VirtualAllocEx(m_process->get_handle(), NULL, 
            m_dllFullPath.size() * 2, MEM_COMMIT, PAGE_READWRITE);
        if (remoteBuff == nullptr) {
            DDLOG(WARNING, "VirtualAllocEx failure");
            break;
        }

        SIZE_T written = 0;
        if (!::WriteProcessMemory(m_process->get_handle(), remoteBuff, 
            m_dllFullPath.data(), m_dllFullPath.size() * 2, &written) ||
            written == 0) {
            DDLOG(WARNING, "WriteProcessMemory failure");
            break;
        }

        u8 oldEnterPointHead[12] {};
        SIZE_T readed = 0;
        if (!::ReadProcessMemory(m_process->get_handle(), &LoadLibraryW,
            oldEnterPointHead, sizeof(oldEnterPointHead), &readed) || readed == 0) {
            DDLOG(WARNING, "ReadProcessMemory failure");
            break;
        }

        written = 0;
        if (!::WriteProcessMemory(m_process->get_handle(), LoadLibraryW,
            &LoadLibraryW, sizeof(oldEnterPointHead), &written) ||
            written == 0) {
            DDLOG(WARNING, "WriteProcessMemory failure");
            break;
        }

        // 在目标进程执行
        hRemoteThread = ::CreateRemoteThread(m_process->get_handle(), NULL, 0, 
            (LPTHREAD_START_ROUTINE)LoadLibraryW, remoteBuff, 0, NULL);
        if (hRemoteThread == NULL) {
            DDLOG_LASTERROR();
            return false;
        }
        ::WaitForSingleObject(hRemoteThread, (DWORD)waitTime);

        written = 0;
        if (!::WriteProcessMemory(m_process->get_handle(), LoadLibraryW,
            oldEnterPointHead, sizeof(oldEnterPointHead), &written) ||
            written == 0) {
            DDLOG(WARNING, "WriteProcessMemory failure");
            break;
        }
        result = true;
    } while (0);

    if (remoteBuff != NULL) {
        ::VirtualFreeEx(m_process->get_handle(), remoteBuff, 0, MEM_RELEASE);
    }

    if (hRemoteThread != NULL) {
        ::CloseHandle(hRemoteThread);
    }
    return result;
}

bool ddclassic_syringeex::uninject_dll(u32 waitTime)
{
    return ddsyringe::uninject_dll(m_process, m_dllFullPath, waitTime);
}

} // namespace NSP_DD
