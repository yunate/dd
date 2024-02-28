#ifndef ddbase_str_ddstr_h_
#define ddbase_str_ddstr_h_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include <stdarg.h>
#include <string>

namespace NSP_DD {

class ddstr;
class ddstr_
{
    friend ddstr;
private:
    static void format(const char* format, std::string& out, va_list args);
    static void format(const wchar_t* format, std::wstring& out, va_list args);
};

class ddstr
{
public:
    template<class _Elem, class F>
    inline static const _Elem* parse_until(const _Elem* str, F func)
    {
        DDASSERT(str != nullptr);
        while (*str != 0 && !func(str)){
            ++str;
        }
        return str;
    }

public:
    inline static std::string format(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        std::string out;
        ddstr_::format(format, out, args);
        va_end(args);
        return std::move(out);
    }
    inline static std::wstring format(const wchar_t* format, ...)
    {
        va_list args;
        va_start(args, format);
        std::wstring out;
        ddstr_::format(format, out, args);
        va_end(args);
        return std::move(out);
    }

    ///////////////////////////////编码转换部分///////////////////////////////////////////
    // std::mbsrtowcs setlocale(LC_CTYPE, "");
    // std::wstring_convert<std::codecvt_utf8<wchar_t> >
    ///////////////////////////////////////////////////////////////////////////////////

    // UTF16和UTF8互相转换
    static bool utf16_8(const std::wstring& src, std::string& des);
    static std::string utf16_8(const std::wstring& src);
    static bool utf8_16(const std::string& src, std::wstring& des);
    static std::wstring utf8_16(const std::string& src);

    // 其他编码页(以多字符编码方式)和UTF16互转
    // code_page: https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
    //            eg: gb2312 -> 936
    static bool to_utf16(const std::string& src, std::wstring& des, u32 code_page);
    static std::wstring to_utf16(const std::string& src, u32 code_page);
    static bool utf16_to(const std::wstring& src, std::string& des, u32 code_page);
    static std::string utf16_to(const std::wstring& src, u32 code_page);

    // 其他编码页(以多字符编码方式)和UTF8互转
    static bool to_utf8(const std::string& src, std::string& des, u32 code_page);
    static std::string to_utf8(const std::string& src, u32 code_page);
    static bool utf8_to(const std::string& src, std::string& des, u32 code_page);
    static std::string utf8_to(const std::string& src, u32 code_page);

    // ansi 编码和 UTF16 编码相互转换
    // system_ansi 为true 表示当前系统的的ANSI, 为false表示当前程序的ANSI, 使用setlocale函数设置当前程序的ANSI
    static bool utf16_ansi(const std::wstring& src, std::string& des, bool system_ansi = true);
    static std::string utf16_ansi(const std::wstring& src, bool system_ansi = true);
    static bool ansi_utf16(const std::string& src, std::wstring& des, bool system_ansi = true);
    static std::wstring ansi_utf16(const std::string& src, bool system_ansi = true);

    // ansi 编码和 UTF8 编码相互转换
    static bool utf8_ansi(const std::string& src, std::string& des);
    static std::string utf8_ansi(const std::string& src);
    static bool ansi_utf8(const std::string& src, std::string& des);
    static std::string ansi_utf8(const std::string& src);

    ///////////////////////////////c字符串处理部分///////////////////////////////////////////
    /** 字符串转小写
    @param [in] src 源
    */
    static void to_lower(char* src);
    static void to_lower(wchar_t* src);
    static void to_lower(std::string& src);
    static void to_lower(std::wstring& src);
    static std::string lower(const char* src);
    static std::wstring lower(const wchar_t* src);

    /** 字符串转大写
    @param [in] src 源
    */
    static void to_upper(char* src);
    static void to_upper(wchar_t* src);
    static void to_upper(std::string& src);
    static void to_upper(std::wstring& src);
    static std::string upper(const char* src);
    static std::wstring upper(const wchar_t* src);

    /** 字符串较
    @param [in] src 源
    @param [in] find 比较
    @param [in] n 个数
    @return 相等 0; src大 >0; find大 <0
    */
    static s32 strcmp(const char* src, const char* find);
    static s32 strcmp(const wchar_t* src, const wchar_t* find);
    static s32 strncmp(const char* src, const char* find, u32 n);
    static s32 strncmp(const wchar_t* src, const wchar_t* find, u32 n);

    /** 以...开头/结尾
    @param [in] src 源
    @param [in] find 为空字符串是返回true
    @return src 是否以find结尾
    */
    static bool endwith(const char* src, const char* find);
    static bool endwith(const wchar_t* src, const wchar_t* find);
    static bool beginwith(const char* src, const char* find);
    static bool beginwith(const wchar_t* src, const wchar_t* find);

    /** 字符串查找
    @param [in] src 源
    @param [in] find 为空字符串是返回 src
    @return src 找到 find, 是, 返回首次出现的地址; 否nullptr
    */
    static const char* strstr(const char* src, const char* find);
    static const wchar_t* strstr(const wchar_t* src, const wchar_t* find);

    /** 字符串查找 '*' 代表零个或者多个任意字符；'?' 代表一个字符（UNICODE中文是两个字符）
    @param [in] src 源
    @param [in] find
    @return 是否匹配
    */
    static bool strwildcard(const char* src, const char* find);
    static bool strwildcard(const wchar_t* src, const wchar_t* find);

    /** 去除字符串首位匹配的字符
    @param [in] src 源
    @param [in] trim 匹配字符(串)
    */
    static void trim(std::string& src);
    static void trim(std::string& src, char trim);
    static void trim(std::string& src, const std::vector<char>& trims);
    static void trim(std::string& src, const std::string& trim);
    static void trim(std::wstring& src);
    static void trim(std::wstring& src, wchar_t trim);
    static void trim(std::wstring& src, const std::vector<wchar_t>& trims);
    static void trim(std::wstring& src, const std::wstring& trim);

    /** 字符串分割
    @param [in] src 源
    @param [in] find 分割子, 调用者保证不为空字符串
    @param [out] out 结果, 不包含空字符串
    */
    static void split(const char* str, const char* find, std::vector<std::string>& out);
    static void split(const wchar_t* str, const wchar_t* find, std::vector<std::wstring> &out);

    /** 字符串替换
    @param [in] src 源, 调用者保证不为空字符串
    @param [in] find 查找目标, 调用者保证不为空字符串
    @param [in] replacer 替换为
    @return 替换后的字符串
    */
    static std::string replace(const char* src, const char* find, const char* replacer);
    static std::wstring replace(const wchar_t* src, const wchar_t* find, const wchar_t* replacer);

    /** 在buff中查找finder, src 和 finder 为空时返回-1
    @param [in] src 源
    @param [in] finder 查找目标, 大小不能超过1M
    @return 如果没有找到返回-1, 否则返回其下标
    */
    static s32 buff_find(const ddbuff& src, const ddbuff& finder);

    /** 替换src中的finder为replacer
    @param [in] src 源
    @param [in] finder 查找目标
    @param [in] replacer 替换
    @param [out] out 输出
    @return 如果没有找到返回false, 否则返回true
    */
    static bool buff_replace(const ddbuff& src, const ddbuff& finder, const ddbuff& replacer, ddbuff& out);

    /** 替换src中的finder为replacer
    @param [in] src 源
    @param [in] finder/replacer 查找和替换, 不会重复替换
                比如 "abcdefg" [("abcd", "xyz"), ("xy", "ss")] 结果是xyzefg 而不是sszefg
    @param [out] out 输出
    @return 返回最后一次找到的下标加上finder的长度, 比如 "abcdefg" [("abcd", "xyz"), ("xy", "ss")] 返回4
    */
    static s32 buff_replace_ex(const ddbuff& src, const std::vector<ddbuff>& finder, const std::vector<ddbuff>& replacer, ddbuff& out);
};

} // namespace NSP_DD
#endif // ddbase_str_ddstr_h_
