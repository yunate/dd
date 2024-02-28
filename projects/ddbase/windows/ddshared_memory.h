#ifndef ddbase_windows_ddshared_memory_h_
#define ddbase_windows_ddshared_memory_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/windows/ddprocess.h"

#include <atomic>
#include <windows.h>

namespace NSP_DD {
class ddshared_memory
{
    DDNO_COPY_MOVE(ddshared_memory);
public:
    ddshared_memory();
    ~ddshared_memory();

    u8* get_buff();
    bool init(u32 size, const std::wstring& name);
    void close();

protected:
    u32 m_size = 0;
    std::wstring m_name;

protected:
    void* m_mapview = nullptr;
    HANDLE m_map = nullptr;
};

class ddasync_shared_memory
{
    DDNO_COPY_MOVE(ddasync_shared_memory);
public:
    ddasync_shared_memory();
    ~ddasync_shared_memory();
    bool init(u32 size, const std::wstring& name);
    u8* lockbuff(u32 waitTime = 3000);
    void unlock();
    void close();

protected:
    ddshared_memory* m_shared_memory = nullptr;
    ddprocess_mutex* m_mutex = nullptr;
};

} // namespace NSP_DD
#endif // ddbase_windows_ddshared_memory_h_

