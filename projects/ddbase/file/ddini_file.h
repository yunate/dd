#ifndef ddbase_file_ddini_file_h_
#define ddbase_file_ddini_file_h_

#include "ddbase/dddef.h"
#include <memory>
#include <string>
#include <vector>

namespace NSP_DD {

class ddini_file;

// ini 文件的读写等
class ddini_file
{
public:
    static ddini_file* create_obj(const std::wstring& sPath, bool bAlwaysCreate = false);
private:
    ddini_file(const std::wstring& sPath);

public:
    /** 在指定section_name下添加/修改一对记录
    @param [in] section_name 哪一个section_name，如果不存在则创建
    @param [in] key 如果存在则修改
    @param [in] value
    @return 是否成功, 如果section_name key value 任何一个为空则返回false
    */
    bool add_key(const std::wstring& section_name, const std::wstring& key, const std::wstring& value);

    /** 在指定section_name下删除一对记录
    @param [in] section_name
    @param [in] key
    @return 是否成功, 如果section_name key任何一个为空则返回false
   */
    bool delete_key(const std::wstring& section_name, const std::wstring& key);

    /** 在指定section_name
    @param [in] section_name
    @return 是否成功, 如果section_name 为空则返回false; 如果section_name不存在则返回true
    */
    bool delete_section(const std::wstring& section_name);

    bool get_all_section_name(std::vector<std::wstring>& section_names);
    bool get_all_key_name(const std::wstring& section_name, std::vector<std::wstring>& skey_names);
    bool get_value(const std::wstring& section_name, const std::wstring& key_name, std::wstring& value);

private:
    /** 全路径
   */
    std::wstring    m_sFileFullPath;
};
} // namespace NSP_DD
#endif // ddbase_file_ddini_file_h_
