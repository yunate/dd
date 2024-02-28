#ifndef ddbase_ddrandom_h_
#define ddbase_ddrandom_h_

#include "ddbase/dddef.h"
#include <windows.h>

namespace NSP_DD {

// 随机管理类
class ddrandom
{
public:
    /** 产生随机数
    @param [in] min 包含
    @param [in] max 包含
    @param [out] out 结果
    @return 是否成功
    */
    static bool get_rand_number(int min, int max, int& out);
};

//////////////////////////////////////GUID////////////////////////////////////
// eg: guidStr:"{431ECE1A-B4EB-4C15-8899-CAB52D13A06A}"
class ddguid
{
public:
    static bool ddstr_guid(const std::wstring& guidStr, ::GUID& guid);
    static bool ddstr_guid(const std::string& guidStr, ::GUID& guid);
    static bool ddguid_str(const ::GUID& guid, std::wstring& guidStr);
    static bool ddguid_str(const ::GUID& guid, std::string& guidStr);
    // 生成guid
    static bool generate_guid(std::string& guid);
    static bool generate_guid(std::wstring& guid);
};

} // namespace NSP_DD
#endif // ddbase_ddrandom_h_
