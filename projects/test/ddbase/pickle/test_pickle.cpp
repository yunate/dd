
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddtime.h"

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <windows.h>

struct aggregate
{
    std::string str;
    std::wstring wstr;
    std::vector<std::string> strs;
    std::map<std::string, std::string> map;
    std::unordered_map<std::string, std::string> umap;
    std::set<std::string> set;
    std::unordered_set<std::string> uset;
};
#if !_HAS_CXX17
DDPICKLE_TRAITS_GEN(aggregate, str, wstr, strs, map, umap, set, uset);
#endif

void test_pickle_auto()
{
    aggregate a;
    a.str = "str";
    a.wstr = L"str";
    a.strs = { "str", "str" };
    a.map = { {"str", "str"}, {"str", "str"} };
    a.umap = { {"str", "str"}, {"str", "str"} };
    a.set = { "str", "str" };
    a.uset = { "str", "str" };

    NSP_DD::ddpickle p1;
    p1 << a;
    aggregate a1;
    p1 >> a1;

    DDASSERT(a.str == a1.str);
    DDASSERT(a.wstr == a1.wstr);
    DDASSERT(a.strs == a1.strs);
    DDASSERT(a.map == a1.map);
    DDASSERT(a.umap == a1.umap);
    DDASSERT(a.set == a1.set);
    DDASSERT(a.uset == a1.uset);
}

void test_pickle_write_read()
{
    int b1 = 1;
    NSP_DD::u16 b2 = 2;
    NSP_DD::u32 b3 = 3;
    NSP_DD::u64 b4 = 4;
    float b5 = 5.0f;
    double b6 = 6.0;
    std::string b7 = "bcd";
    std::string b8 = "efgh";
    std::string b9 = "ijklm";
    std::string b10 = "nopqr";
    std::vector<std::string> b11 = { "st", "uvw", "xyzb", "bcdef" };
    std::map<int, int> b12 = { {3,4}, {5,6}, {1,2} };
    std::string b13 = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
    abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";

    NSP_DD::ddpickle p1;
    p1 << b1;
    p1 << b2;
    p1 << b3;
    p1 << b4;
    p1 << b5;
    p1 << b6;
    p1 << b7;
    p1 << b8;
    p1 << b9;
    p1 << b10;
    p1 << b11;
    p1 << b12;
    p1 << b13;

    //////////////////////////////////////////////////////////////////////////
    int a1 = 0;
    NSP_DD::u16 a2 = 0;
    NSP_DD::u32 a3 = 0;
    NSP_DD::u64 a4 = 0;
    float a5 = 0.0f;
    double a6 = 0.0;
    std::string a7;
    std::string a8;
    std::string a9;
    std::string a10;
    std::vector<std::string> a11;
    std::map<int, int> a12;
    std::string a13;
    p1 >> a1;
    p1 >> a2;
    p1 >> a3;
    p1 >> a4;
    p1 >> a5;
    p1 >> a6;
    p1 >> a7;
    p1 >> a8;
    p1 >> a9;
    p1 >> a10;
    p1 >> a11;
    p1 >> a12;
    p1 >> a13;
    DDASSERT(a1 == b1);
    DDASSERT(a2 == b2);
    DDASSERT(a3 == b3);
    DDASSERT(a4 == b4);
    DDASSERT(a5 == b5);
    DDASSERT(a6 == b6);
    DDASSERT(a7 == b7);
    DDASSERT(a8 == b8);
    DDASSERT(a9 == b9);
    DDASSERT(a10 == b10);
    DDASSERT(a11 == b11);
    DDASSERT(a12 == b12);
}

void test_pickle_pod()
{
    struct pod
    {
        double a;
        float b;
        NSP_DD::u8 c;
        NSP_DD::u16 d;
        NSP_DD::u32 e;
        NSP_DD::u64 f;
        NSP_DD::s8 g;
        NSP_DD::s16 h;
        NSP_DD::s32 i;
        NSP_DD::s64 j;
    };
    static_assert(std::is_trivially_copyable<pod>::value, "struct pod is not pod");
    pod a { 1.0, 1.0f, 1, 2, 3, 4, 5, 6, 7, 8 };
    NSP_DD::ddpickle p1;
    p1 << a;
    pod a1;
    p1 >> a1;
    DDASSERT(::memcmp(&a1, &a, sizeof(pod)) == 0);
}

struct base
{
    std::string a;
    std::string b;
    std::string c;
};
#if !_HAS_CXX17
DDPICKLE_TRAITS_GEN(base, a, b, c);
#endif

struct son : base
{
    std::string a;
    std::string b;
    std::string c;
};

DDPICKLE_TRAITS_GEN_EX(son, DDEXPEND(base), a, b, c);

void test_pickle_inherit()
{
    son a;
    a.base::a = "basea";
    a.base::b = "baseB";
    a.base::c = "baseC";
    a.a = "sona";
    a.b = "sonB";
    a.c = "sonC";
    NSP_DD::ddpickle p1;
    p1 << a;
    son a1;
    p1 >> a1;
    DDASSERT(a.a == a1.a);
    DDASSERT(a.b == a1.b);
    DDASSERT(a.c == a1.c);
    DDASSERT(a.base::a == a1.base::a);
    DDASSERT(a.base::b == a1.base::b);
    DDASSERT(a.base::c == a1.base::c);
}

namespace NSP_DD {
DDTEST(test_pickle, auto)
{
    test_pickle_auto();
    test_pickle_write_read();
    test_pickle_pod();
    test_pickle_inherit();
}
} // namespace NSP_DD
