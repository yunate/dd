#ifndef ddhook_dd_winapi_hooker_h_
#define ddhook_dd_winapi_hooker_h_

#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

namespace NSP_DD {

/** 修改函数头使其跳转
x64:
    // mov rax, new_proc
    new_head_bytes[0] = '\x48';
    new_head_bytes[1] = '\xb8';
    (void)::memcpy_s(new_head_bytes + 2, 8, &new_proc, 8);

    // jmp rax
    new_head_bytes[10] = '\xff';
    new_head_bytes[11] = '\xe0';

win32:
    // jmp offset
    new_head_bytes[0] = '\xE9';
    u32 offset = (u32)new_proc - (u32)raw_proc_addr - 5;
    (void)::memcpy_s(new_head_bytes + 1, 4, &offset, 4);
*/
struct dd_winapi_hooker
{
public:
    static bool regist_hooker(dd_winapi_hooker** hooker, HMODULE api_dll, const std::string& api_name, void* new_proc, void* user_long_ptr);

public:
    bool init(HMODULE api_dll, const std::string& api_name, void* new_proc, void* user_long);
    ~dd_winapi_hooker();
    bool hook();
    void unhook();
    bool use_raw_head_bytes() const;
    bool use_new_head_bytes() const;
    bool copy_head_bytes();

    bool hooked = false;
    void* user_long = nullptr;
    HANDLE current_process = NULL;
    void* raw_proc_addr = NULL;

#ifdef _WIN64
    char raw_head_bytes[12] = { 0 };
    char new_head_bytes[12] = { 0 };
#else
    char raw_head_bytes[5] = { 0 };
    char new_head_bytes[5] = { 0 };
#endif
};
using sp_hooker = std::shared_ptr<dd_winapi_hooker>;

class ddhooker_manager
{
    using hooker_map = std::unordered_map<std::string, sp_hooker>;
public:
    /** 注册一个hooker
    @param [in] key 用户自定义字符串, 用于map的唯一key
    @param [in] api_name 所要hook的api名称
    @param [in] api_dll 所要hook的api所在dll, 比如MessageBoxA对应User32.dll
    @param [in] new_proc hook回调函数
    @param [in] user_long 用户指针, 可以传入对象的地址
    @return 是否成功
    */
    bool regist_hooker(const std::string& key, const std::string& api_name, HMODULE api_dll, void* new_proc, void* user_long);
    void unregist_hooker(const std::string& key);
    sp_hooker get_hooker(const std::string& key);
    void exec_raw(const std::string& key, std::function<void()> func);
private:
    hooker_map m_hooker_map;
};

#define DDHOOKER_MANAGER ddsingleton<ddhooker_manager>::get_instance()

} // namespace NSP_DD
#endif // ddhook_dd_winapi_hooker_h_