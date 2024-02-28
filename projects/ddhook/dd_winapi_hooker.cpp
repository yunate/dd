#include "ddhook/stdafx.h"
#include "ddhook/dd_winapi_hooker.h"
namespace NSP_DD {
bool dd_winapi_hooker::use_raw_head_bytes() const
{
    SIZE_T written = 0;
    return (::WriteProcessMemory(current_process, (LPVOID)raw_proc_addr, raw_head_bytes, sizeof(raw_head_bytes), &written) == TRUE);
}

bool dd_winapi_hooker::use_new_head_bytes() const
{
    SIZE_T written = 0;
    return (::WriteProcessMemory(current_process, (LPVOID)raw_proc_addr, new_head_bytes, sizeof(new_head_bytes), &written) == TRUE);
}

bool dd_winapi_hooker::copy_head_bytes()
{
    SIZE_T readed = 0;
    return (::ReadProcessMemory(current_process, raw_proc_addr, raw_head_bytes, sizeof(raw_head_bytes), &readed) == TRUE);
}

bool dd_winapi_hooker::init(HMODULE api_dll, const std::string& api_name, void* new_proc, void* user_long_ptr)
{
    this->user_long = user_long_ptr;

    // current process
    current_process = ::GetCurrentProcess();

    // raw proc's addr
    raw_proc_addr = ::GetProcAddress(api_dll, api_name.c_str());
    if (raw_proc_addr == NULL) {
        return false;
    }

    // new proc head bytes
#ifdef _WIN64
// mov rax, new_proc
    new_head_bytes[0] = '\x48';
    new_head_bytes[1] = '\xb8';
    (void)::memcpy_s(new_head_bytes + 2, 8, &new_proc, 8);

    // jmp rax
    new_head_bytes[10] = '\xff';
    new_head_bytes[11] = '\xe0';
#else
// jmp offset
    new_head_bytes[0] = '\xE9';
    u32 offset = (u32)new_proc - (u32)raw_proc_addr - 5;
    (void)::memcpy_s(new_head_bytes + 1, 4, &offset, 4);
#endif

    return true;
}

dd_winapi_hooker::~dd_winapi_hooker()
{
    unhook();
}

void dd_winapi_hooker::unhook()
{
    if (hooked) {
        (void)use_raw_head_bytes();
        hooked = false;
    }
}

bool dd_winapi_hooker::hook()
{
    // rawroc head bytes
    if (!copy_head_bytes()) {
        return false;
    }

    if (!use_new_head_bytes()) {
        return false;
    }

    hooked = true;
    return true;
}

bool dd_winapi_hooker::regist_hooker(dd_winapi_hooker** hooker, HMODULE api_dll, const std::string& api_name, void* new_proc, void* user_long)
{
    *hooker = new dd_winapi_hooker();
    if (*hooker == nullptr) {
        return false;
    }
    if (!(*hooker)->init(api_dll, api_name, new_proc, user_long) || !(*hooker)->hook()) {
        delete (*hooker);
        *hooker = nullptr;
        return false;
    }
    return true;
}

bool ddhooker_manager::regist_hooker(const std::string& key, const std::string& api_name, HMODULE api_dll, void* new_proc, void* user_long)
{
    dd_winapi_hooker* hooker = nullptr;
    if (!dd_winapi_hooker::regist_hooker(&hooker, api_dll, api_name, new_proc, user_long)) {
        return false;
    }

    unregist_hooker(key);
    m_hooker_map[key] = std::shared_ptr<dd_winapi_hooker>(hooker);
    return true;
}

void ddhooker_manager::unregist_hooker(const std::string& key)
{
    if (m_hooker_map.find(key) != m_hooker_map.end()) {
        m_hooker_map.erase(key);
    }
}

sp_hooker ddhooker_manager::get_hooker(const std::string& key)
{
    auto it = m_hooker_map.find(key);
    if (it != m_hooker_map.end()) {
        return it->second;
    }
    return nullptr;
}

void ddhooker_manager::exec_raw(const std::string& key, std::function<void()> func)
{
    auto hooker = get_hooker(key);
    if (hooker != nullptr && func != nullptr) {
        hooker->use_raw_head_bytes();
        func();
        hooker->use_new_head_bytes();
    }
}

} // namespace NSP_DD
