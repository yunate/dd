
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddtimer.h"

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <windows.h>

namespace NSP_DD {

void my_pickele()
{
    ddpickle p1;
    p1 << 1;
    p1 << (u16)3;
    p1 << (u32)4;
    p1 << (u64)5;
    p1 << 6.0f;
    p1 << (double)7.0;

    p1 << "bcd";
    p1 << "efgh";
    p1 << "ijklm";
    p1 << std::string("nopqr");
    p1 << std::vector<std::string>{"st", "uvw", "xyza", "bcdef"};
    p1 << std::map<int, int>{ {3, 4}, { 5,6 }, { 1,2 } };
    p1 << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\
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
    //////////////////////////////////////////////////////////////////////////
    int a = 0; // 1
    u16 c = 0; // 3
    u32 d = 0; // 4
    u64 e = 0; // 5
    float f = 0.0f; // 6
    double g = 0.0; // 7
    std::string i; // "bcd"
    std::string j; // "efgh"
    std::string k; // "ijklm"
    std::string l; // "nopqr"
    std::vector<std::string> m; // { "st", "uvw", "xyza", "bcdef" }
    std::map<int, int>n; // { {3,4}, {5,6}, {1,2} }
    p1 >> a;
    p1 >> c;
    p1 >> d;
    p1 >> e;
    p1 >> f;
    p1 >> g;
    p1 >> i;
    p1 >> j;
    p1 >> k;
    p1 >> l;
    p1 >> m;
    p1 >> n;
    std::string o;
    p1 >> o;
}

 DDTEST(test_pickle1, u1)
 {
     ddtimer timer;
     timer.reset();
     for (int i = 0; i < 1000000; ++i) {
         my_pickele();
     }
     std::cout << "my_pickele " << timer.get_time_pass() / 1000 << std::endl;
 
     timer.reset();
     for (int i = 0; i < 1000000; ++i) {
         // ipc_pickle();
     }
     std::cout << "ipc_pickle " << timer.get_time_pass() / 1000 << std::endl;
 }

DDTEST(test_pickle2, pod)
{
    struct Pod
    {
        double a;
        float b;
        u8 c;
        u16 d;
        u32 e;
        u64 f;
        s8 g;
        s16 h;
        s32 i;
        s64 j;
    };
    static_assert(std::is_pod<Pod>::value, "Pod is not pod");
    Pod pod{ 1.0, 1.0f, 1, 2, 3, 4, 5, 6, 7, 8 };
    ddpickle p1;
    p1 << pod;
    Pod podr;
    p1 >> podr;
}

struct PickleNotPod
{
    double a = 0;
    float b = 0;
    std::string c;
};

template<>
class ddpickle_helper<PickleNotPod, ddcontainer_traits::none>
{
public:
    static bool write(ddpickle& pck, const PickleNotPod& r)
    {
        bool success = true;
        success &= pck << r.a;
        success &= pck << r.b;
        success &= pck << r.c;
        return success;
    }

    static bool read(ddpickle& reader, PickleNotPod& r)
    {
        bool success = true;
        success &= reader >> r.a;
        success &= reader >> r.b;
        success &= reader >> r.c;
        return success;
    }
};

DDTEST(test_pickle3, PickleNotPod)
{
    PickleNotPod s{ 1.0, 1.0f, "abc" };
    ddpickle p1;
    p1 << s;
    PickleNotPod ss;
    p1 >> ss;
}

struct PickleNotPod20
{
    float a = 1;
    float a1 = 1;
    float a2 = 1;
    float a3 = 1;
    float a4 = 1;
    float a5 = 1;
    float a6 = 1;
    float a7 = 1;
    float a8 = 1;
    float a9 = 1;
    float a10 = 1;
    float a11 = 1;
    float a12 = 1;
    float a13 = 1;
    float a14 = 1;
    float a15 = 1;
    float a16 = 1;
    float a17 = 1;
    float a18 = 1;
    float a19 = 1;
};
DDPICKLE_TRAITS_GEN(PickleNotPod20, a, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19)
DDTEST(test_pickle4, PickleNotPod20)
{
    PickleNotPod20 s{ 2.0, 2.0, 2.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 , 1.0 };
    ddpickle p1;
    p1 << s;
    PickleNotPod20 ss;
    p1 >> ss;
}

struct PickleNotPod25 : public PickleNotPod20
{
    char a = 1;
    char a1 = 1;
    char a2 = 1;
    char a3 = 1;
    std::string a4 = "abced";
};
DDPICKLE_TRAITS_GEN_EX(PickleNotPod25, DDEXPEND(PickleNotPod20), a, a1, a2, a3, a4)
DDTEST(test_pickle5, PickleNotPod25)
{
    PickleNotPod25 s;
    s.a4 = "ddd";
    ddpickle p1;
    p1 << s;
    PickleNotPod25 ss;
    p1 >> ss;
}

struct PickleNotPod0
{
};
DDPICKLE_TRAITS_GEN(PickleNotPod0)
DDTEST(test_pickle6, PickleNotPod0)
{
    PickleNotPod0 s;
    ddpickle p1;
    p1 << s;
    PickleNotPod0 ss;
    p1 >> ss;
}

struct PickleNotPod0_b : public PickleNotPod0, PickleNotPod25
{
};
DDPICKLE_TRAITS_GEN_EX(PickleNotPod0_b, DDEXPEND(PickleNotPod0, PickleNotPod25))
DDTEST(test_pickle7, PickleNotPod0_b)
{
    PickleNotPod0_b s;
    s.PickleNotPod25::a = 100;
    s.PickleNotPod25::a4 = "lllllllllllllllllllllllllllllllll";
    ddpickle p1;
    p1 << s;
    PickleNotPod0_b ss;
    p1 >> ss;
}
} // namespace NSP_DD
