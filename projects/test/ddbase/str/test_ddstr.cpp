#include "test/stdafx.h"

#include "ddbase/str/ddstr.h"
#include "ddbase/str/ddurl.hpp"
#include "ddbase/ddio.h"

#include "ddbase/ddtest_case_factory.h"

namespace NSP_DD {

DDTEST(test_ddstr, str_fmt)
{
    std::string fmt = ddstr::format("%d.%d.%d", 1, 2, 3);
    std::wstring wfmt = ddstr::format(L"%d.%d.%d", 1, 2, 3);

}

#if defined(__cpp_lib_char8_t)
static std::string from_u8string(const std::u8string& s) {
    return std::string(s.begin(), s.end());
}
#else
static std::string from_u8string(const std::string& s) {
    return s;
}
#endif

DDTEST(test_ddstr, utf16_8)
{
    // gbk 你好
    std::string hello_gbk = "\xC4\xE3\xBA\xC3";
    // utf8 你好
    std::string hello_utf8 = from_u8string(u8"\u4F60\u597D");
    // utf16 你好
    std::wstring hello_utf16 = L"\u4F60\u597D";

    // UTF16和UTF8互相转换
    {
        std::string utf8 = ddstr::utf16_8(hello_utf16);
        DDASSERT(utf8 == hello_utf8);
        std::wstring utf16 = ddstr::utf8_16(hello_utf8);
        DDASSERT(utf16 == hello_utf16);
    }

    // 其他编码页(以多字符编码方式)和UTF16互转
    {
        std::wstring utf16 = ddstr::to_utf16(hello_gbk, 936);
        DDASSERT(utf16 == hello_utf16);
        std::string gbk = ddstr::utf16_to(hello_utf16, 936);
        DDASSERT(gbk == hello_gbk);
    }

    // 其他编码页(以多字符编码方式)和UTF8互转
    {
        std::string gbk = ddstr::utf8_to(hello_utf8, 936);
        DDASSERT(gbk == hello_gbk);
        std::string utf8 = ddstr::to_utf8(hello_gbk, 936);
        DDASSERT(utf8 == hello_utf8);
    }

    std::wstring str = L"hello test";
    std::string utf8Str;
    ddstr::utf16_8(str, utf8Str);
}

DDTEST(test_ddstr, url_parser)
{
    std::vector<std::string> urls;
    urls.push_back("http://www.baidu.com:20/");
    urls.push_back("http://www.baidu.com:20/?#aa");
    urls.push_back("http:\\\\FF:123@www.baidu.com:80\\wget.apx\\/dd\\/?name=ydh#anchor");
    urls.push_back("http:\\\\:123@www.baidu.com:83\\wget.apx//dd/?name=ydh#anchor");
    urls.push_back("http:\\\\FF:@www.baidu.com:82\\wget.apx\\\\dd/?name=ydh#anchor");
    urls.push_back("http:\\\\FF@www.baidu.com:81\\wget.apx/dd\\\\?name=ydh#anchor");
    urls.push_back("http:\\\\FF@www.baidu.com:\\wget.apx/dd//?name=ydh#anchor");
    urls.push_back("http:\\\\FF@www.baidu.com\\wget.apx/dd/\\?name=ydh#anchor");
    urls.push_back("http:\\\\FF@\\wget.apx/dd/?name=ydh#anchor");
    urls.push_back("http://www.baidu.com/wget.apx/dd/?name=ydh#anchor");
    urls.push_back("http://www.baidu.com#anchor?name=ydh/wget.apx/dd/");
    urls.push_back("http://www.baidu.com/wget.apx/#anchor");
    urls.push_back("http://www.baidu.com/wget.apx/dd/?name=ydh#anchor");
    urls.push_back("http://www.baidu.com/wget.apx/dd/?name=ydh#anchor");
    urls.push_back("http://www.baidu.com/wget.apx/dd?name=ydh#");
    urls.push_back("http://www.baidu.com/wget.apx/dd?name=ydh");
    urls.push_back("http://www.baidu.com/wget.apx/dd/cc?");
    urls.push_back("http://www.baidu.com/wget.apx/");
    urls.push_back("http://ydh:123@w/wget.apx");
    urls.push_back("http:///wget.apx");
    urls.push_back("http://@/wget.apx");
    urls.push_back("http://:@/wget.apx");
    urls.push_back("http://:@:/wget.apx");
    urls.push_back("http://:/wget.apx");
    urls.push_back("http://www.baidu.com:20/");
    urls.push_back("http://www.baidu.com:20");
    urls.push_back("http://www.baidu.com:");
    urls.push_back("http://www.baidu.com/");
    urls.push_back("http:///?#");
    urls.push_back("http://w/?dada#");
    urls.push_back("http://w/?dada#daada");
    urls.push_back("http://w/#dada?daada");
    urls.push_back("http://w/#dada#ab");
    urls.push_back("http://");
    urls.push_back("http:/");

    for (auto& it : urls)
    {
        ddcout(gray) << "\r\n" << it.c_str() << "\r\n";
        ddurl url;
        parse_url(it, url);
        ddcout(green) << "norml:" << url.get_formated_url();

        ddurl urlEx;
        parse_url_regex(it, urlEx);
        ddcout(green) << "\r\nregex:" << urlEx.get_formated_url() << "\r\n";
    }
}

DDTEST(test_ddstr_case, str_lower)
{
    {
        std::string fmt = "";
        ddstr::to_lower(fmt);
    }

    {
        std::string fmt = "abcdefg";
        ddstr::to_lower(fmt);
    }

    {
        std::string fmt = "ABcdef";
        ddstr::to_lower(fmt);
    }

    {
        std::string fmt = "ABcdef与";
        ddstr::to_lower(fmt);
    }

    {
        std::wstring fmt = L"";
        ddstr::to_lower(fmt);
    }

    {
        std::wstring fmt = L"abcdefg";
        ddstr::to_lower(fmt);
    }

    {
        std::wstring fmt = L"ABcdef";
        ddstr::to_lower(fmt);
    }

    {
        std::wstring fmt = L"ABcdef与";
        ddstr::to_lower(fmt);
    }
}

DDTEST(test_ddstr_case, str_upper)
{
    {
        std::string fmt = "";
        ddstr::to_upper(fmt);
    }

    {
        std::string fmt = "abcdefg";
        ddstr::to_upper(fmt);
    }

    {
        std::string fmt = "ABcdef";
        ddstr::to_upper(fmt);
    }

    {
        std::string fmt = "ABcdef与";
        ddstr::to_upper(fmt);
    }

    {
        std::wstring fmt = L"";
        ddstr::to_upper(fmt);
    }

    {
        std::wstring fmt = L"abcdefg";
        ddstr::to_upper(fmt);
    }

    {
        std::wstring fmt = L"ABcdef";
        ddstr::to_upper(fmt);
    }

    {
        std::wstring fmt = L"ABcdef与";
        ddstr::to_upper(fmt);
    }
}

DDTEST(test_ddstr2, strcmp)
{
    std::vector<std::tuple<std::string, std::string, bool>> test
    {
        {"", "", true},
        {"我", "我", true},
        {"我是", "我是", true},
        {"我是谁", "我是", false},
        {"我是", "我是谁", false},
        {"我是", "", false},
        {"", "我是", false},
        {"我是谁", "猫狗鹅", false},
    };
    std::vector<std::tuple<std::string, std::string, bool>> test1
    {
        {"", "", true},
        {"a", "a", true},
        {"ab", "ab", true},
        {"abc", "ab", false},
        {"ab", "abc", false},
        {"ab", "", false},
        {"", "ab", false},
        {"ab", "cdef", false},
    };

    for (const auto& it : test) {
        if (std::get<2>(it) != (ddstr::strcmp(std::get<0>(it).c_str(), std::get<1>(it).c_str()) == 0)) {
            DDASSERT(false);
        }
    }

    for (const auto& it : test1) {
        if (std::get<2>(it) != (ddstr::strcmp(std::get<0>(it).c_str(), std::get<1>(it).c_str()) == 0)) {
            DDASSERT(false);
        }
    }
}
DDTEST(test_ddstr2, strcmpw)
{
    std::vector<std::tuple<std::wstring, std::wstring, bool>> test
    {
        {L"", L"", true},
        {L"我", L"我", true},
        {L"我是", L"我是", true},
        {L"我是谁", L"我是", false},
        {L"我是", L"我是谁", false},
        {L"我是", L"", false},
        {L"", L"我是", false},
        {L"我是谁", L"猫狗鹅", false},
    };
    std::vector<std::tuple<std::wstring, std::wstring, bool>> test1
    {
        {L"", L"", true},
        {L"a", L"a", true},
        {L"ab", L"ab", true},
        {L"abc", L"ab", false},
        {L"ab", L"abc", false},
        {L"ab", L"", false},
        {L"", L"ab", false},
        {L"ab", L"cdef", false},
    };

    for (const auto& it : test) {
        if (std::get<2>(it) != (ddstr::strcmp(std::get<0>(it).c_str(), std::get<1>(it).c_str()) == 0)) {
            DDASSERT(false);
        }
    }

    for (const auto& it : test1) {
        if (std::get<2>(it) != (ddstr::strcmp(std::get<0>(it).c_str(), std::get<1>(it).c_str()) == 0)) {
            DDASSERT(false);
        }
    }
}

DDTEST(test_ddstr2, strncmp)
{
    std::vector<std::tuple<std::string, std::string, bool>> test
    {
        {"", "", true},
        {"我", "我", true},
        {"我是", "我是", true},
        {"我是谁", "我是", false},
        {"我是", "我是谁", false},
        {"我是", "", false},
        {"", "我是", false},
        {"我是谁", "猫狗鹅", false},
    };
    std::vector<std::tuple<std::string, std::string, bool>> test1
    {
        {"", "", true},
        {"a", "a", true},
        {"ab", "ab", true},
        {"abc", "ab", false},
        {"ab", "abc", false},
        {"ab", "", false},
        {"", "ab", false},
        {"ab", "cdef", false},
    };

    for (const auto& it : test) {
        std::string src = std::get<0>(it);
        std::string find = std::get<1>(it);
        for (u32 i = 0; i <= (u32)find.length(); ++i) {
            if (std::get<2>(it) != (ddstr::strncmp(src.c_str(), find.c_str(), i) == 0)) {
                // DDASSERT(false);
            }
        }
    }

    for (const auto& it : test1) {
        std::string src = std::get<0>(it);
        std::string find = std::get<1>(it);
        for (u32 i = 0; i <= (u32)find.length(); ++i) {
            if (std::get<2>(it) != (ddstr::strncmp(src.c_str(), find.c_str(), i) == 0)) {
                // DDASSERT(false);
            }
        }
    }
}

DDTEST(test_ddstr2, beginwith)
{
    std::vector<std::tuple<std::string, std::string, bool>> test
    {
        {"", "", true},
        {"我", "我", true},
        {"我是", "我是", true},
        {"我是谁", "我是", true},
        {"我是", "我是谁", false},
        {"我是", "", true},
        {"", "我是", false},
        {"我是谁", "猫狗鹅", false},
    };
    std::vector<std::tuple<std::string, std::string, bool>> test1
    {
        {"", "", true},
        {"a", "a", true},
        {"ab", "ab", true},
        {"abc", "ab", true},
        {"ab", "abc", false},
        {"ab", "", true},
        {"", "ab", false},
        {"ab", "cdef", false},
    };

    for (const auto& it : test) {
        if (std::get<2>(it) != ddstr::beginwith(std::get<0>(it).c_str(), std::get<1>(it).c_str())) {
            DDASSERT(false);
        }
    }

    for (const auto& it : test1) {
        if (std::get<2>(it) != ddstr::beginwith(std::get<0>(it).c_str(), std::get<1>(it).c_str())) {
            DDASSERT(false);
        }
    }
}

DDTEST(test_ddstr2, endwith)
{
    std::vector<std::tuple<std::string, std::string, bool>> test
    {
        {"", "", true},
        {"我", "我", true},
        {"我是", "是", true},
        {"我是谁", "我是", false},
        {"我是", "我是谁", false},
        {"我是", "", true},
        {"", "我是", false},
        {"我是谁", "猫狗鹅", false},
    };
    std::vector<std::tuple<std::string, std::string, bool>> test1
    {
        {"", "", true},
        {"a", "a", true},
        {"ab", "b", true},
        {"abc", "ab", false},
        {"ab", "abc", false},
        {"ab", "", true},
        {"", "ab", false},
        {"ab", "cdef", false},
    };

    for (const auto& it : test) {
        if (std::get<2>(it) != ddstr::endwith(std::get<0>(it).c_str(), std::get<1>(it).c_str())) {
            DDASSERT(false);
        }
    }

    for (const auto& it : test1) {
        if (std::get<2>(it) != ddstr::endwith(std::get<0>(it).c_str(), std::get<1>(it).c_str())) {
            DDASSERT(false);
        }
    }
}

DDTEST(test_ddstr3, strstr)
{
    std::vector<std::tuple<std::string, std::string, bool>> test
    {
        {"", "", true},
        {"我", "我", true},
        {"我是", "是", true},
        {"我是谁", "我是", true},
        {"我是", "我是谁", false},
        {"我是", "", true},
        {"", "我是", false},
        {"我是谁", "猫狗鹅", false},
        {"我是谁猫狗鹅", "猫狗", true},
    };
    std::vector<std::tuple<std::string, std::string, bool>> test1
    {
        {"", "", true},
        {"a", "a", true},
        {"ab", "b", true},
        {"abc", "ab", true},
        {"ab", "abc", false},
        {"ab", "", true},
        {"", "ab", false},
        {"abcdef", "c", true},
    };

    for (const auto& it : test) {
        if (std::get<2>(it) != (ddstr::strstr(std::get<0>(it).c_str(), std::get<1>(it).c_str()) != nullptr)) {
            DDASSERT(false);
        }
    }

    for (const auto& it : test1) {
        if (std::get<2>(it) != (ddstr::strstr(std::get<0>(it).c_str(), std::get<1>(it).c_str()) != nullptr)) {
            DDASSERT(false);
        }
    }
}

DDTEST(test_ddstr3, strstrw)
{
    std::vector<std::tuple<std::wstring, std::wstring, bool>> test
    {
        {L"", L"", true},
        {L"我", L"我", true},
        {L"我是", L"是", true},
        {L"我是谁", L"我是", true},
        {L"我是", L"我是谁", false},
        {L"我是", L"", true},
        {L"", L"我是", false},
        {L"我是谁", L"猫狗鹅", false},
        {L"我是谁猫狗鹅", L"猫狗", true},
    };
    std::vector<std::tuple<std::wstring, std::wstring, bool>> test1
    {
        {L"", L"", true},
        {L"a", L"a", true},
        {L"ab", L"b", true},
        {L"abc", L"ab", true},
        {L"ab", L"abc", false},
        {L"ab", L"", true},
        {L"", L"ab", false},
        {L"abcdef", L"c", true},
    };

    for (const auto& it : test) {
        if (std::get<2>(it) != (ddstr::strstr(std::get<0>(it).c_str(), std::get<1>(it).c_str()) != nullptr)) {
            DDASSERT(false);
        }
    }

    for (const auto& it : test1) {
        if (std::get<2>(it) != (ddstr::strstr(std::get<0>(it).c_str(), std::get<1>(it).c_str()) != nullptr)) {
            DDASSERT(false);
        }
    }
}

DDTEST(test_ddstr4, strwildcard)
{
    std::vector<std::tuple<std::wstring, std::wstring, bool>> test
    {
        {L"", L"", true},
        {L"我", L"我", true},
        {L"我是", L"是", false},
        {L"我是谁", L"我*谁", true},
        {L"我是谁", L"我*?谁", true},
        {L"我是", L"我是谁", false},
        {L"我是", L"", false},
        {L"我是谁", L"*?", true},
        {L"我是谁猫狗鹅", L"*?猫*?鹅", true},
    };
    std::vector<std::tuple<std::string, std::string, bool>> test2
    {
        {"我谁", "我*谁", true},
        {"", "", true},
        {"我", "我", true},
        {"我是", "是", false},
        {"我是谁", "我*谁", true},
        {"我是谁", "我*?谁", true},
        {"我是", "我是谁", false},
        {"我是", "", false},
        {"我是谁", "*?", true},
        {"我是谁猫狗鹅", "*?猫*?鹅", true},
    };
    for (const auto& it : test) {
        std::wstring src = std::get<0>(it);
        std::wstring find = std::get<1>(it);
        if (std::get<2>(it) != ddstr::strwildcard(src.c_str(), find.c_str())) {
            DDASSERT(false);
        }
    }
    for (const auto& it : test2) {
        std::string src = std::get<0>(it);
        std::string find = std::get<1>(it);
        if (std::get<2>(it) != ddstr::strwildcard(src.c_str(), find.c_str())) {
            DDASSERT(false);
        }
    }
}

DDTEST(test_ddstr5, split)
{
    {
        std::vector<std::tuple<std::string, std::string, u32>> test2
        {
            {"abc_def_ghi_jkl_mn", "_", 5},
            {"aaaaaaaa", "a", 0},
            {"abcdefg", "a", 1},
            {"bcdefga", "a", 1},
            {"abcdefga", "a", 1},
            {"", "a", 0},
        };

        for (const auto& it : test2) {
            std::string src = std::get<0>(it);
            std::string find = std::get<1>(it);
            std::vector<std::string> out;
            ddstr::split(src.c_str(), find.c_str(), out);
            DDASSERT(out.size() == std::get<2>(it));
        }
    }

    {
        std::vector<std::tuple<std::wstring, std::wstring, u32>> test2
        {
            {L"abc_def_ghi_jkl_mn", L"_", 5},
            {L"aaaaaaaa", L"a", 0},
            {L"abcdefg", L"a", 1},
            {L"bcdefga", L"a", 1},
            {L"abcdefga", L"a", 1},
            {L"", L"a", 0},
        };

        for (const auto& it : test2) {
            std::wstring src = std::get<0>(it);
            std::wstring find = std::get<1>(it);
            std::vector<std::wstring> out;
            ddstr::split(src.c_str(), find.c_str(), out);
            DDASSERT(out.size() == std::get<2>(it));
        }
    }
}

DDTEST(test_ddstr6, parse_until)
{
    std::string str = R"(
abcdefg
hijklmn
opq
rst
uvw
xyz
)";
    std::string spliter(1, '\n');
    std::vector<std::string> out;
    ddstr::split(str.c_str(), spliter.c_str(), out);

    std::vector<std::string> out1;
    const char* src = str.c_str();
    const char* beg = src;
    while (*src != 0) {
        src = ddstr::parse_until(src, [](const char* src) {
            if (*src == '\r' || *src == '\n') {
                return true;
            }
            return false;
        });

        if (src != beg) {
            out1.emplace_back(beg, src - beg);
        }
        while (*src == '\r' || *src == '\n') {
            ++src;
        }
        beg = src;
    }
    if (*beg != 0) {
        out1.emplace_back(beg);
    }

    DDASSERT(out.size() == out1.size());
    for (size_t i = 0; i < out.size(); ++i) {
        DDASSERT(out[i] == out1[i]);
    }
}


DDTEST(test_ddstr7, trim1)
{
    using group = std::tuple<std::string, std::vector<char>, std::string>;
    std::vector<group> tests{
        {"abcfabc", {'a', 'b', 'c'}, "f"},
        {"abcf", {'a', 'b', 'c'}, "f"},
        {"fabc", {'a', 'b', 'c'}, "f"},
        {"abfab", {'a', 'b', 'c'}, "f"},
        {"abcfabc", {}, "abcfabc"},
        {"", {'a', 'b', 'c'}, ""},
    };

    for (const auto& it : tests) {
        std::string src = std::get<0>(it);
        const std::vector<char>& trims = std::get<1>(it);
        const std::string expect = std::get<2>(it);
        ddstr::trim(src, trims);
        DDASSERT(src == expect);
    }
}

DDTEST(test_ddstr7, trim2)
{
    using group = std::tuple<std::string, std::string, std::string>;
    std::vector<group> tests{
        {"abcfabc", "abc", "f"},
        {"abcf", "abc", "f"},
        {"fabc", "abc", "f"},
        {"abfab", "abc", "abfab"},
        {"abcfabc", "", "abcfabc"},
        {" ", "", " "},
    };

    for (const auto& it : tests) {
        std::string src = std::get<0>(it);
        const std::string& trim = std::get<1>(it);
        const std::string expect = std::get<2>(it);
        ddstr::trim(src, trim);
        DDASSERT(src == expect);
    }
}

DDTEST(test_ddstr7, trim3)
{
    using group = std::tuple<std::wstring, std::wstring, std::wstring>;
    std::vector<group> tests{
        {L"abcfabc", L"abc", L"f"},
        {L"abcf", L"abc", L"f"},
        {L"fabc", L"abc", L"f"},
        {L"abfab", L"abc", L"abfab"},
        {L"abcfabc", L"", L"abcfabc"},
        {L" ", L"", L" "},
    };

    for (const auto& it : tests) {
        std::wstring src = std::get<0>(it);
        const std::wstring& trim = std::get<1>(it);
        const std::wstring expect = std::get<2>(it);
        ddstr::trim(src, trim);
        DDASSERT(src == expect);
    }
}

DDTEST(test_ddstr8, replacer)
{
    {
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> test2
        {
            {"abc_def_ghi_jkl_mn", "_", "+", "abc+def+ghi+jkl+mn"},
            {"abc_def_ghi_jkl_mn", "_", "", "abcdefghijklmn"},
            {"______", "_", "+", "++++++"},
            {"_bcdefg", "_", "+", "+bcdefg"},
            {"bcdefg_", "_", "+", "bcdefg+"},
            {"_bcdefg_", "_", "+", "+bcdefg+"},
            {"", "_", "+", ""},
            {"abc", "_", "+", "abc"},
        };

        for (const auto& it : test2) {
            std::string src = std::get<0>(it);
            std::string find = std::get<1>(it);
            std::string replace = std::get<2>(it);
            std::string except = std::get<3>(it);
            std::string result = ddstr::replace(src.c_str(), find.c_str(), replace.c_str());
            DDASSERT(except == result);
        }
    }

    {
        std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring>> test2
        {
            {L"abc_def_ghi_jkl_mn", L"_", L"+", L"abc+def+ghi+jkl+mn"},
            {L"abc_def_ghi_jkl_mn", L"_", L"", L"abcdefghijklmn"},
            {L"______", L"_", L"+", L"++++++"},
            {L"_bcdefg", L"_", L"+", L"+bcdefg"},
            {L"bcdefg_", L"_", L"+", L"bcdefg+"},
            {L"_bcdefg_", L"_", L"+", L"+bcdefg+"},
            {L"", L"_", L"+", L""},
            {L"abc", L"_", L"+", L"abc"},
        };

        for (const auto& it : test2) {
            std::wstring src = std::get<0>(it);
            std::wstring find = std::get<1>(it);
            std::wstring replace = std::get<2>(it);
            std::wstring except = std::get<3>(it);
            std::wstring result = ddstr::replace(src.c_str(), find.c_str(), replace.c_str());
            DDASSERT(except == result);
        }
    }
}

DDTEST(test_ddstr10, replacer)
{
    {
        std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring>> test2
        {
            {L"aaaaaaaaaa", L"a", L"b", L"bbbbbbbbbb"}
        };

        for (const auto& it : test2) {
            std::wstring src = std::get<0>(it);
            std::wstring find = std::get<1>(it);
            std::wstring replace = std::get<2>(it);
            std::wstring except = std::get<3>(it);
            std::wstring result = ddstr::replace(src.c_str(), find.c_str(), replace.c_str());
            DDASSERT(except == result);
        }
    }
}

DDTEST(test_ddstr9, buff_finder)
{
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 1,2,3,4 };
        DDASSERT(ddstr::buff_find(src, finder) == 1);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{0, 1,2,3,4 };
        DDASSERT(ddstr::buff_find(src, finder) == 0);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ };
        DDASSERT(ddstr::buff_find(src, finder) == -1);
    }
    {
        ddbuff src{ };
        ddbuff finder{ 0, 1,2,3,4 };
        DDASSERT(ddstr::buff_find(src, finder) == -1);
    }
    {
        ddbuff src{ 0 };
        ddbuff finder{ 0, 1,2,3,4 };
        DDASSERT(ddstr::buff_find(src, finder) == -1);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 0 };
        DDASSERT(ddstr::buff_find(src, finder) == 0);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 7,8,9 };
        DDASSERT(ddstr::buff_find(src, finder) == 7);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 9 };
        DDASSERT(ddstr::buff_find(src, finder) == 9);
    }
}

DDTEST(test_ddstr10, buff_replace)
{
    // 中间
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 1,2,3,4 };
        ddbuff replaceer{ 1,2,3,4 };
        ddbuff out;
        ddbuff exp{ 0,1,2,3,4,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 1,2,3,4 };
        ddbuff replaceer{ 1,2,4 };
        ddbuff out;
        ddbuff exp{ 0,1,2,4,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 1,2,3,4 };
        ddbuff replaceer{ 1,2,3,4,5 };
        ddbuff out;
        ddbuff exp{ 0,1,2,3,4,5,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    // 开头
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 0,1,2,3,4 };
        ddbuff replaceer{ 0, 1,2,3,4 };
        ddbuff out;
        ddbuff exp{ 0,1,2,3,4,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 0,1,2,3,4 };
        ddbuff replaceer{ 0, 1,3,4 };
        ddbuff out;
        ddbuff exp{ 0,1,3,4,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 0,1,2,3,4 };
        ddbuff replaceer{ 0, 1,2,3,4,6 };
        ddbuff out;
        ddbuff exp{ 0,1,2,3,4,6,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    // 末尾
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 6,7,8,9 };
        ddbuff replaceer{ 6,7,8,9};
        ddbuff out;
        ddbuff exp{ 0,1,2,3,4,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 6,7,8,9 };
        ddbuff replaceer{ 6,7,8,9,0 };
        ddbuff out;
        ddbuff exp{ 0,1,2,3,4,5,6,7,8,9,0 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 6,7,8,9 };
        ddbuff replaceer{ 6,7,8};
        ddbuff out;
        ddbuff exp{ 0,1,2,3,4,5,6,7,8};
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    // 全
    {
        ddbuff src{ 0,0,0,0,0,0,0,0,0,0 };
        ddbuff finder{ 0 };
        ddbuff replaceer{ 1 };
        ddbuff out;
        ddbuff exp{ 1,1,1,1,1,1,1,1,1,1 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    {
        ddbuff src{ 0,0,0,0,0,0,0,0,0,0 };
        ddbuff finder{ 0 };
        ddbuff replaceer{ 1,1 };
        ddbuff out;
        ddbuff exp{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    // 多个
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 1,2,3 };
        ddbuff replaceer{ 4,5,6 };
        ddbuff out;
        ddbuff exp{ 0,4,5,6,4,5,6,7,8,9,0,4,5,6,4,5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == true);
        DDASSERT(out == exp);
    }

    // 无
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9 };
        ddbuff finder{ 1,2,3,'a'};
        ddbuff replaceer{ 4,5,6 };
        ddbuff out;
        ddbuff exp = src;
        DDASSERT(ddstr::buff_replace(src, finder, replaceer, out) == false);
        DDASSERT(out == exp);
    }
}

DDTEST(test_ddstr11, buff_replace)
{
    // 开头
    {
        ddbuff src{ 0,1,2,3,4,5,6,7,8,9 };
        std::vector<ddbuff> finder{ {0,1},{1},{2},{3},{4} };
        std::vector<ddbuff> replaceer{ {'a'}, {'b'}, {'c'}, {'d'}, {'e'}};
        ddbuff out;
        ddbuff exp{ 'a', 'c','d', 'e',5,6,7,8,9};
        DDASSERT(ddstr::buff_replace_ex(src, finder, replaceer, out) == (s32)exp.size());
        DDASSERT(out == exp);
    }

    // 中间
    {
        ddbuff src{ 'x', 'x', 'x', 'x', 0,1,2,3,4,5,6,7,8,9 };
        std::vector<ddbuff> finder{ {0,1},{1},{2},{3},{4} };
        std::vector<ddbuff> replaceer{ {'a'}, {'b'}, {'c'}, {'d'}, {'e'} };
        ddbuff out;
        ddbuff exp{ 'x', 'x', 'x', 'x', 'a', 'c','d', 'e',5,6,7,8,9 };
        DDASSERT(ddstr::buff_replace_ex(src, finder, replaceer, out) == (s32)exp.size());
        DDASSERT(out == exp);
    }

    // 末尾
    {
        ddbuff src{ 5,6,7,8,9,0,1,2,3,4 };
        std::vector<ddbuff> finder{ {0,1},{1},{2},{3},{4} };
        std::vector<ddbuff> replaceer{ {'a'}, {'b'}, {'c'}, {'d'}, {'e'} };
        ddbuff out;
        ddbuff exp{ 5,6,7,8,9, 'a', 'c','d', 'e' };
        DDASSERT(ddstr::buff_replace_ex(src, finder, replaceer, out) == (s32)exp.size());
        DDASSERT(out == exp);
    }

    // 无
    {
        ddbuff src{ 5,6,7,8,9,0,1,2,3,4 };
        std::vector<ddbuff> finder{ {0,1, 1},{1,1, 1},{2,1, 1},{3,1, 1},{4,1, 1} };
        std::vector<ddbuff> replaceer{ {'a'}, {'b'}, {'c'}, {'d'}, {'e'} };
        ddbuff out;
        ddbuff exp = src;
        DDASSERT(ddstr::buff_replace_ex(src, finder, replaceer, out) == (s32)exp.size());
        DDASSERT(out == exp);
    }
}

DDTEST(test_ddstr12, str_format)
{
    {
        std::string line(1024 - 1, 'a');
        std::string formated_line = ddstr::format("%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }
    {
        std::string line(1024 * 5 - 1, 'a');
        std::string formated_line = ddstr::format("%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }
    {
        std::string line(1024 * 5, 'a');
        std::string formated_line = ddstr::format("%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }
    {
        std::string line(1024 * 6, 'a');
        std::string formated_line = ddstr::format("%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }

    {
        std::wstring line(1024 - 1, L'a');
        std::wstring formated_line = ddstr::format(L"%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }
    {
        std::wstring line(1024 * 5 - 1, L'a');
        std::wstring formated_line = ddstr::format(L"%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }
    {
        std::wstring line(1024 * 5, L'a');
        std::wstring formated_line = ddstr::format(L"%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }
    {
        std::wstring line(1024 * 6, L'a');
        std::wstring formated_line = ddstr::format(L"%s", line.c_str());
        DDASSERT(formated_line.size() == line.size());
        DDASSERT(formated_line == line);
    }
}
} // namespace NSP_DD
