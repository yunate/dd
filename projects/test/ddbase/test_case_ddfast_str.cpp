
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/str/ddfast_str.hpp"

namespace NSP_DD {
DDTEST(test_ddfast_str, trim)
{
    ddfast_str fast_str1("f   ");
    fast_str1.trim();
    DDASSERT(fast_str1.length() == 1);

    ddfast_str fast_str2("   f");
    fast_str2.trim();
    DDASSERT(fast_str2.length() == 1);

    ddfast_str fast_str3("");
    fast_str3.trim();
    DDASSERT(fast_str3.length() == 0);

    ddfast_str fast_str4(" f ");
    fast_str4.trim();
    DDASSERT(fast_str4.length() == 1);

    ddfast_str fast_str5("  ");
    fast_str5.trim();
    DDASSERT(fast_str5.length() == 0);
}

DDTEST(test_ddfast_str, trim1)
{
    using group = std::tuple<std::string, std::vector<char>, std::string>;
    std::vector<group> tests {
        {"abcfabc", {'a', 'b', 'c'}, "f"},
        {"abcf", {'a', 'b', 'c'}, "f"},
        {"fabc", {'a', 'b', 'c'}, "f"},
        {"abfab", {'a', 'b', 'c'}, "f"},
        {"abcfabc", {}, "abcfabc"},
        {"", {'a', 'b', 'c'}, ""},
    };

    for (const auto& it : tests) {
        ddfast_str fast_str(std::get<0>(it));
        const std::vector<char>& trims = std::get<1>(it);
        const std::string expect = std::get<2>(it);
        fast_str.trim(trims);
        DDASSERT(fast_str.to_str() == expect);
    }
}

DDTEST(test_ddfast_str, trim2)
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
        ddfast_str fast_str(std::get<0>(it));
        const std::string& trim = std::get<1>(it);
        const std::string expect = std::get<2>(it);
        fast_str.trim(trim);
        DDASSERT(fast_str.to_str() == expect);
    }
}

DDTEST(test_ddfast_str, beg_with)
{
    ddfast_str fast_str("abc_def_ghi_jkl_mn");
    DDASSERT(fast_str.beg_with(""));
    DDASSERT(fast_str.beg_with("abc_"));
    DDASSERT(!fast_str.beg_with("abcd"));

    ddfast_str fast_str1("");
    DDASSERT(!fast_str1.beg_with("abc_"));
    DDASSERT(fast_str1.beg_with(""));
}

DDTEST(test_ddfast_str, end_with)
{
    ddfast_str fast_str("abc_def_ghi_jkl_mn");
    DDASSERT(fast_str.end_with(""));
    DDASSERT(fast_str.end_with("mn"));
    DDASSERT(!fast_str.end_with("abcd"));

    ddfast_str fast_str1("");
    DDASSERT(!fast_str1.end_with("abc_"));
    DDASSERT(fast_str1.end_with(""));
}

DDTEST(test_ddfast_str, sub_str)
{
    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        ddfast_str sub = fast_str.sub_str(0, fast_str.length());
        DDASSERT(sub == fast_str);
    }

    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        ddfast_str sub = fast_str.sub_str(0);
        DDASSERT(sub == fast_str);
    }

    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        ddfast_str sub = fast_str.sub_str(fast_str.find("d"));
        DDASSERT(sub == "def_ghi_jkl_mn");
    }

    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        ddfast_str sub = fast_str.sub_str(fast_str.find("d"), 3);
        DDASSERT(sub == "def");
    }

    {
        ddfast_str fast_str1("");
        ddfast_str sub1 = fast_str1.sub_str(0, fast_str1.length());
        DDASSERT(sub1 == fast_str1);
    }
}

DDTEST(test_ddfast_str, to_sub_str)
{
    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        fast_str.to_sub_str(0, fast_str.length());
        ddfast_str sub(fast_str);
        DDASSERT(sub == fast_str);
    }

    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        fast_str.to_sub_str(0);
        ddfast_str sub(fast_str);
        DDASSERT(sub == fast_str);
    }

    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        fast_str.to_sub_str(fast_str.find("d"));
        ddfast_str sub(fast_str);
        DDASSERT(sub == "def_ghi_jkl_mn");
    }

    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        fast_str.to_sub_str(fast_str.find("d"), 3);
        ddfast_str sub(fast_str);
        DDASSERT(sub == "def");
    }

    {
        ddfast_str fast_str1("");
        fast_str1.to_sub_str(0, fast_str1.length());
        ddfast_str sub1(fast_str1);
        DDASSERT(sub1 == fast_str1);
    }
}

DDTEST(test_ddfast_str, find)
{
    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        DDASSERT(fast_str.find('a') == 0);
        DDASSERT(fast_str.find("a") == 0);
        DDASSERT(fast_str.find('n') == fast_str.length() - 1);
        DDASSERT(fast_str.find("n") == fast_str.length() - 1);
        DDASSERT(fast_str.find("abc") == 0);
        DDASSERT(fast_str.find("mn") == fast_str.length() - 2);
        DDASSERT(fast_str.find("abc_def_ghi_jkl_mn") == 0);
        DDASSERT(fast_str.find("") == 0);
        DDASSERT(fast_str.find("ff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find("abc_def_ghi_jkl_mnff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find('x') == ddfast_str::xnpos);
    }

    {
        ddfast_str fast_str("");
        DDASSERT(fast_str.find('a') == ddfast_str::xnpos);
        DDASSERT(fast_str.find("a") == ddfast_str::xnpos);
        DDASSERT(fast_str.find('n') == ddfast_str::xnpos);
        DDASSERT(fast_str.find("n") == ddfast_str::xnpos);
        DDASSERT(fast_str.find("abc") == ddfast_str::xnpos);
        DDASSERT(fast_str.find("mn") == ddfast_str::xnpos);
        DDASSERT(fast_str.find("abc_def_ghi_jkl_mn") == ddfast_str::xnpos);
        DDASSERT(fast_str.find("") == 0);
        DDASSERT(fast_str.find("ff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find("abc_def_ghi_jkl_mnff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find('x') == ddfast_str::xnpos);
    }
}

DDTEST(test_ddfast_str, find_last_of)
{
    {
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        DDASSERT(fast_str.find_last_of('a') == 0);
        DDASSERT(fast_str.find_last_of("a") == 0);
        DDASSERT(fast_str.find_last_of('n') == fast_str.length() - 1);
        DDASSERT(fast_str.find_last_of("n") == fast_str.length() - 1);
        DDASSERT(fast_str.find_last_of("abc") == 0);
        DDASSERT(fast_str.find_last_of("mn") == fast_str.length() - 2);
        DDASSERT(fast_str.find_last_of("abc_def_ghi_jkl_mn") == 0);
        DDASSERT(fast_str.find_last_of("") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("ff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("abc_def_ghi_jkl_mnff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of('x') == ddfast_str::xnpos);
    }

    {
        ddfast_str fast_str("");
        DDASSERT(fast_str.find_last_of('a') == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("a") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of('n') == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("n") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("abc") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("mn") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("abc_def_ghi_jkl_mn") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("ff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of("abc_def_ghi_jkl_mnff") == ddfast_str::xnpos);
        DDASSERT(fast_str.find_last_of('x') == ddfast_str::xnpos);
    }
}

DDTEST(test_ddfast_str, split)
{
    {
        std::vector<ddfast_str> out;
        ddfast_str fast_str("abc_def_ghi_jkl_mn");
        fast_str.split("_", out);
        DDASSERT(out.size() == 5);

        out.clear();
        fast_str.split("__", out);
        DDASSERT(out.size() == 1);

        out.clear();
        fast_str.split("abc_def_ghi_jkl_mn", out);
        DDASSERT(out.size() == 0);

        out.clear();
        fast_str.split("abc_def_ghi_jkl_mnd", out);
        DDASSERT(out.size() == 1);
    }

    {
        std::vector<ddfast_str> out;
        ddfast_str fast_str("");
        fast_str.split("_", out);
        DDASSERT(out.size() == 0);

        out.clear();
        fast_str.split("__", out);
        DDASSERT(out.size() == 0);

        out.clear();
        fast_str.split("abc_def_ghi_jkl_mn", out);
        DDASSERT(out.size() == 0);

        out.clear();
        fast_str.split("abc_def_ghi_jkl_mnd", out);
        DDASSERT(out.size() == 0);
    }
}


DDTEST(test_ddfast_str, split1)
{
    using group = std::tuple<std::string, std::vector<std::string>, std::vector<std::string>>;
    std::vector<group> tests {
        {"abc_def_ghi_jkl_mn", {"_"}, {"abc", "def", "ghi", "jkl", "mn"}},
        {"abc_def_ghi_jkl_mn", {"__"}, {"abc_def_ghi_jkl_mn"}},
        {"abc_def_ghi_jkl_mn", {"abc_def_ghi_jkl_mn"}, {}},
        {"abc_def_ghi_jkl_mn", {"abc_def_ghi_jkl_mnd"}, {"abc_def_ghi_jkl_mn"}},

        {"", {"_"}, {}},
        {"", {"__"}, {}},
        {"", {"abc_def_ghi_jkl_mn"}, {}},
        {"", {"abc_def_ghi_jkl_mnd"}, {}},
    }; 
    for (const auto& it : tests) {
        const std::string& src = std::get<0>(it);
        const std::vector<std::string>& spliters = std::get<1>(it);
        const std::vector<std::string>& expect = std::get<2>(it);
        ddfast_str fast_str(src);
        std::vector<ddfast_str> out;
        fast_str.split_ex([&spliters](const ddfast_str& src)
            -> std::tuple<size_t, size_t> {
            for (const auto& cmp : spliters) {
                size_t pos = src.find(cmp);
                if (pos != ddfast_str::xnpos) {
                    return { pos, cmp.length()};
                }
            }
            return { ddfast_str::xnpos, 0};
        }, out);
        DDASSERT(out.size() == expect.size());
        for (size_t i = 0; i < out.size(); ++i) {
            DDASSERT(out[i] == expect[i]);
        }
    }
}
} // namespace NSP_DD
