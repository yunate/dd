#ifndef ddbase_dderror_code_h_
#define ddbase_dderror_code_h_

#include "ddbase/dddef.h"
#include <Windows.h>

namespace NSP_DD {

// windows 第二十九位设置为1(0x20000000)时候表示是用户自定义
class dderror_code
{
public:
    enum : DWORD {
        // 表示成功
        ok = 0x20000000,

        //////////////////////// common ////////////////////////
        // 未知错误
        unexpected,

        // 内存不足
        out_of_memory,

        // 超时
        time_out,

        // 参数不匹配
        param_mismatch,

        // 表示参数为nullptr
        param_nullptr,

        // 表示参数是一个无效句柄
        param_invalid_handle,

        // 表示初始化失败或未调用 init 函数
        init_failure = 0x20000000 + 1025,

        // 重复初始化
        init_repeat,

        // 未找到 key
        key_not_find,

        // key 已经存在
        key_exist,

        // buff 内存太小
        buff_size_not_enough,

        // 句柄已经被关闭
        handle_closed,

        // 操作进行中
        operate_pending,

        // 重复操作,比如上一次的操作已经成功了,不用再次调用了
        operate_repeat,

        // 对象已经被释放
        object_released,

        //////////////////////// iocp ////////////////////////
        inner_task_run,

        //////////////////////// network ////////////////////////
        invalid_url,

        end,
    };

    // error_code:
    //  1. https://learn.microsoft.com/en-us/windows/win32/debug/last-error-code
    //  2. dderror_code
    static DWORD get_last_error();
    static void set_last_error(DWORD error_code);
    static std::string get_error_msga(DWORD error_code);
    static std::string get_last_error_msga();
    static std::wstring get_error_msgw(DWORD error_code);
    static std::wstring get_last_error_msgw();
};

///////////////////////////////////////////dderror_code_guard///////////////////////////////////////////
class dderror_code_guard
{
public:
    dderror_code_guard();
    ~dderror_code_guard();

private:
    DWORD m_error_code = 0;
};

} // namespace NSP_DD
#endif // ddbase_dderror_code_h_

