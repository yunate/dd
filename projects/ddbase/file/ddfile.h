#ifndef ddbase_file_ddfile_h_
#define ddbase_file_ddfile_h_
#include "ddbase/dddef.h"

#include "ddbase/ddnocopyable.hpp"
#include <functional>
#include <string>
#include <windows.h>

namespace NSP_DD {
enum class ddfile_type
{
    utf8,
    ansi,
    // utf8 和 ansi 两种编码的文件都没有文件头, 难以区分
    utf8_or_ansi,
    // utf8 bom 编码的文件 文件头 { 0xef, 0xbb, 0xbf }
    utf8bom,
    // utf16 le 编码的文件 文件头 { 0xff, 0xfe }
    utf16le,
    // utf16 le 编码的文件 文件头 { 0xfe, 0xff }
    utf16be,
    // 未知文件类型, 文件打开失败或其他错误
    unknown
};

class ddfile
{
    DDNO_COPY_MOVE(ddfile);
private:
    // use create_or_open() to create.
    ddfile();

public:
    ~ddfile();

    /** 创建文件, 文件soffset为0
    @param [in] path 文件路径
    @return 是否成功, 如果文件已经存在则打开
    */
    static ddfile* create_or_open(const std::wstring& path, bool read_only = false);

    /** 创建一个文件, 并检查文件头.
        文件soffset为head_checker_size;
        如果文件size为0,则写入文件;
        如果文件size不为0且其头部几个字符和head_checker中的不相同, 返回nullptr.
    @param [in] path 文件路径
    @param [in] head_checker 目标文件头内容, 内存块由调用者维护
    @param [in] head_checker_size head_checker内存块大小
    @return 成功返回ddfile指针, 否则返回nullptr
    */
    static ddfile* create_file_with_head_checker(const std::wstring& path, const u8* head_checker, s32 head_checker_size, bool read_only = false);

    // 创建一个utf16 le 编码的文件 使用read_linew读取文件
    // 文件soffset为2;
    static ddfile* create_usc2_file(const std::wstring& path, bool read_only = false);

    // 创建一个utf16 be  编码的文件 使用read_linew读取文件
    static ddfile* create_usc2_file_be(const std::wstring& path, bool read_only = false);

    // 创建一个UTF8 编码的文件 使用read_linea读取文件
    // 文件soffset为0;
    static ddfile* create_utf8_file(const std::wstring& path, bool read_only = false);

    // 创建一个UTF8bom 编码的文件 使用read_linea读取文件
    // 文件soffset为3;
    static ddfile* create_utf8bom_file(const std::wstring& path, bool read_only = false);

    // 获取文件类型
    static ddfile_type get_file_type(const std::wstring& path);

    /** 简单判断文件是否是文本文件
        检查文件前1024个字符, 如果出现(00 8*)或者偶数位出现0 这样的返回false
    @param [in] src 源文件
    */
    static bool try_check_is_txt_file(ddfile* src);

    /** 在文本中发现finder
    @param [in] src 源文件
    @param [in] finder 查找的buff
    @return 如果找不到返回-1, 否则返回下标
    */
    static s64 file_find(ddfile* src, const ddbuff& finder);

    /** 在文本中发现finder并替换为replacer
    @param [in] src/dst 源/目标文件
    @param [in] finder/replacer 查找和替换, 不会重复替换
            比如 "abcdefg" [("abcd", "xyz"), ("xy", "ss")] 结果是xyzefg 而不是sszefg
    */
    static bool file_replace(ddfile* src, ddfile* dst, const std::vector<ddbuff>& finder, const std::vector<ddbuff>& replacer);

    /** 写文件
    @param [in] buff 输入内存块
    @param [in] size 内存块大小
    @return 真正写入大小
    */
    s32 write(const u8* buff, s32 size);

    /** write函数通常将数据写入操作系统定期写入磁盘的内部缓冲区, 此函数将指定文件的缓冲信息写入磁盘.
    @return 是否成功
    */
    bool flush();

    /** 读文件
    @param [out] buff 输入内存块, 调用者分配/释放内存
    @param [in] size 内存块大小
    @return 真正读出大小
    */
    s32 read(u8* buff, s32 size);

    /** 读文件, 读一行知道'\n'
    @param [out] line 读出的一行文件
    @return 是否成功
    */
    bool read_linew(std::wstring& line);
    bool read_linea(std::string& line);

    /** 读文件直到 until 返回true
    @param [in] until callback
    */
    void read_untila(std::function<bool(char ch)> until);
    void read_untilw(std::function<bool(wchar_t ch)> until);

    /** 重置文件大小
        文件soffset为size;
    @param [in] size 新的文件大小
    @return 是否成功
    */
    bool resize(s64 size = 0);

    /** seek
    @param [in] offset 偏移, 为0时候seek到起始点, -1 时候seek到end.
    @return 是否成功
    */
    bool seek(s64 offset);

    /** 获得当前文件偏移
    @return 文件偏移
    */
    s64 current_offset();

    /** 获得文件大小
    @return 文件大小
    */
    s64 file_size();

    // 获得文件大小, 不打开文件的方式
    static s64 file_size(const std::wstring& full_path);

    /** 获得文件全路径
    @return 文件全路径
    */
    const std::wstring& get_fullpath();
private:
    HANDLE m_file_handle = NULL;
    std::wstring m_fullpath;
};
} // namespace NSP_DD
#endif // ddbase_file_ddfile_h_
