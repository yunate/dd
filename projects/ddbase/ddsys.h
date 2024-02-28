
#ifndef ddbase_ddsys_h_
#define ddbase_ddsys_h_

#include "ddbase/dddef.h"
#include <thread>
#include <windows.h>

namespace NSP_DD {

inline u32 get_thread_count() 
{

    // 通过std提供的方法获取
    u32 count = std::thread::hardware_concurrency();

    if (count != 0) {
        return count;
    }

    // 如果获取不到，再调用winapi尝试获取
    ::SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    count = (u32)info.dwNumberOfProcessors;

    // 如果还是获取不到，尝试 
    // WMIC CPU Get NumberOfCores,NumberOfLogicalProcessors /Format:List > ./cpucore.txt 
    // 命令获取，但是这儿因为这个命令可能要初始化WMIC，还要涉及文件读写而引入其他库，所以就不去尝试了

    if (count != 0) {
        return count;
    }

    // 如果还是获取不到，直接返回1
    return 1;
}

} // namespace NSP_DD
#endif // ddbase_ddsys_h_
