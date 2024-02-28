#ifndef ddbase_ddsingle_limited_h_
#define ddbase_ddsingle_limited_h_

#include "ddbase/dddef.h"

#include <vector>
#include <windows.h>
namespace NSP_DD {

// 尝试创建并持有某些对象，用来限制实例唯一
class ddsingle_limited
{
public:
    ~ddsingle_limited()
    {
        ::CloseHandle(m_handles);
    }

    bool test(const std::wstring& mutexName)
    {
        bool exit = false;
        HANDLE mutex = ::CreateMutexW(nullptr, FALSE, mutexName.c_str());
        if (mutex == nullptr) {
            return exit;
        }

        if (ERROR_ALREADY_EXISTS != ::GetLastError()) {
            exit = true;
            return true;
        }

        (void)::CloseHandle(mutex);
        return exit;
    }

    // 返回false，持有失败
    bool try_hold_mutex(const std::wstring& mutexName)
    {
        HANDLE mutex = ::CreateMutexW(nullptr, FALSE, mutexName.c_str());
        if (mutex == nullptr) {
            return false;
        }

        if (ERROR_ALREADY_EXISTS != ::GetLastError())
        {
            m_handles = mutex;
            return true;
        }

        (void)::CloseHandle(mutex);
        return false;
    }

private:
    HANDLE m_handles = NULL;
};

} // namespace NSP_DD
#endif // ddbase_ddsingle_limited_h_
