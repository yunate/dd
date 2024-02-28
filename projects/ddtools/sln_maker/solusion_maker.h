#ifndef ddtools_sln_maker_solusion_maker_h_
#define ddtools_sln_maker_solusion_maker_h_

#include "ddbase/ddmini_include.h"
#include <string>

namespace NSP_DD {

enum class ddproject_type
{
    exe = 0, // Application
    lib, // StaticLibrary
    dll, // DynamicLibrary 
};

enum class DDMT_D
{
    MT = 1,
    MD = 2,
};

struct ddsolusion_maker_param
{
    std::wstring base_dir; // 不能为空
    std::wstring sln_name; // 空字符串表示不创建, 如果.sln存在则将.vcxproj添加到.sln
    std::wstring proj_name; // project 名称, 不能为空
    ddproject_type proj_type = ddproject_type::exe; // 不能为空
    u32 mt_d = (u32)DDMT_D::MT | (u32)DDMT_D::MD;
    bool use_vcpkg = false;
};

class ddsolusion_maker
{
public:
    ddsolusion_maker(const ddsolusion_maker_param& param);
    bool make();

private:
    bool make_base_dir();
    bool copy_templete_and_replace();
    bool make_sln();
    bool add_proj_2_sln();
    bool copy_user_props();
private:
    ddsolusion_maker_param m_param;
    std::wstring m_templete;
    std::string m_proj_guid;
};

} // namespace NSP_DD
#endif // ddtools_sln_maker_solusion_maker_h_
