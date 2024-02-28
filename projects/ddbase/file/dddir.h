#ifndef ddbase_file_dddir_h_
#define ddbase_file_dddir_h_

#include "ddbase/dddef.h"

#include <functional>
#include <string>
#include <vector>

namespace NSP_DD {

class dddir
{
public:
    /** 是否是文件夹
    @param [in] path 路径
    */
    static bool is_dir(const std::wstring& path);

    /** 判断文件或文件夹是否存在
    @param [in] path 路径
    */
    static bool is_path_exist(const std::wstring& file_path);

    /** 创建文件
    @param [in] file_path 文件路径
    */
    static bool create_file(const std::wstring& file_path);

    /** 删除文件(夹)
    @param [in] path 文件(夹)路径
    */
    static bool delete_path(const std::wstring& path);

    /** 创建文件夹
    @param [in] dir_path 文件夹路径
    */
    static bool create_dir(const std::wstring& dir_path);

    /** 创建文件夹(嵌套创建)
    @param [in] dir_path 文件夹路径
    */
    static bool create_dir_ex(const std::wstring& dir_path);

    /** 重命名路径(移动文件)
        如果目标文件(夹)存在则覆盖
        该函数返回后文件移动完成
    @param [in] src_path 原文件(夹)路径
    @param [in] dst_path 目标文件(夹)路径
    */
    static bool rename_path(const std::wstring& src_path, const std::wstring& dst_path);

    /** 复制文件(夹)
        如果目标文件(夹)存在则覆盖
    @param [in] src_path 原文件(夹)路径
    @param [in] dst_path 目标文件(夹)路径
    */
    static bool copy_path(const std::wstring& src_path, const std::wstring& dst_path);

    /** 复制文件(夹)
        如果目标文件(夹)存在则覆盖
    @param [in] src_path 原文件(夹)路径
    @param [in] dst_path 目标文件(夹)路径
    @param [in] exclude_filter 不为nullptr且返回true不复制
    */
    static bool copy_path_ex(
        const std::wstring& src_path,
        const std::wstring& dst_path,
        const std::function<bool(const std::wstring&)>& exclude_filter = nullptr);

    struct enum_dir_info
    {
        std::wstring name;
        bool is_dir = false;
        bool is_hidden = false;
        s64 file_size = 0;
    };
    /** 枚举目录 只枚举第一层子目录
    @param [in] dir_path 目录路径
    @param [in] callBack 回调函数，对每一个路径进行处理
                @param [in] name 文件(夹)名称 full_path = dir_path + name
                @param [in] is_dir 是否是目录
                @return true 停止枚举
    */
    static void enum_dir_first_level(const std::wstring& dir_path, std::vector<dddir::enum_dir_info>& result);
    static void enum_dir_first_level(const std::wstring& dir_path, const std::function<bool(const dddir::enum_dir_info& file_info)>& call_back);

    /** 枚举目录 广度优先
    @param [in] dir_path 目录路径
    @param [in] callBack 回调函数，对每一个路径进行处理
                @param [in] full_path 全路径
                @param [in] is_dir 是否是目录
                @return true 停止枚举
    */
    static void enum_dir(const std::wstring& dir_path, const std::function<bool(const std::wstring& full_path, bool is_dir)>& call_back);

    /** 枚举目录 广度优先
    @param [in] dir_path 目录路径
    @param [in] callBack 回调函数，对每一个路径进行处理
                @param [in] sub_path 子路径, full_path = dir_path + sub_path + name
                @param [in] name 文件(夹)名称
                @param [in] is_dir 是否是目录
                @return true 停止枚举
    */
    static void enum_dir(const std::wstring& dir_path, const std::function<bool(const std::wstring& sub_path, const std::wstring& name, bool is_dir)>& call_back);

    /** 枚举目录 广度优先
    @param [in] dir_path 目录路径
    @param [in] out 输出路径vector
    @param [in] filter 对每一path进行过滤，返回true这放到out结果中，否则丢掉。filter为空时候当作返回true
                @param [in] path 路径
                @param [in] is_dir 是否是目录
                @return true 将path放到out结果，false 丢掉
    */
    static void enum_dir(const std::wstring& dir_path, std::vector<std::wstring>& out, const std::function<bool(const std::wstring& path, bool is_dir)>& filter);
};

} // namespace NSP_DD
#endif // ddbase_file_dddir_h_