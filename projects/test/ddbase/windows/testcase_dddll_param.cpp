#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/dddll_param.h"


namespace NSP_DD {

struct PODD
{
    u8 a;
    u16 b;
    u32 c;
    u64 d;
    s64 d1;
    s32 c1;
    s16 b1;
    s8 a1;
};

DDTEST(testcase_dddll_param, test)
{
    for (int i = 0; i < 100; ++i)
    {
        {
            using T = std::vector<PODD>;
            T src{ {1,2,3,4,5,6,7,8} , {1,2,3,4,5,6,7,8} , {1,2,3,4,5,6,7,8} , {1,2,3,4,5,6,7,8} , {1,2,3,4,5,6,7,8} };
            dddllbuff* dllbuff = DDDLL_TO_BUFF(T, src);
            T dst = DDDLL_FROM_BUFF(T, dllbuff);
            DDASSERT(src.size() == dst.size());
        }

        {
            std::wstring src = L"abcdefg";
            dddllbuff* dllbuff = DDDLL_TO_BUFF(std::wstring, src);
            std::wstring dst = DDDLL_FROM_BUFF(std::wstring, dllbuff);
            DDASSERT(src == dst);
        }

        {
            using T = std::vector<std::wstring>;
            T src{ L"abcdefg"};
            dddllbuff* dllbuff = DDDLL_TO_BUFF(T, src);
            T dst = DDDLL_FROM_BUFF(T, dllbuff);
            DDASSERT(src == dst);
        }

        {
            using T = std::set<std::wstring>;
            T src{ L"abcdefg" , L"abcdefg1" , L"abcdefg2" , L"abcdefg3" , L"abcdefg4" };
            dddllbuff* dllbuff = DDDLL_TO_BUFF(T, src);
            T dst = DDDLL_FROM_BUFF(T, dllbuff);
            DDASSERT(src == dst);
        }

        {
            using T = std::unordered_set<std::wstring>;
            T src{ L"abcdefg" , L"abcdefg1" , L"abcdefg2" , L"abcdefg3" , L"abcdefg4" };
            dddllbuff* dllbuff = DDDLL_TO_BUFF(T, src);
            T dst = DDDLL_FROM_BUFF(T, dllbuff);
            DDASSERT(src == dst);
        }

        {
            using T = std::map<std::wstring, std::wstring>;
            T src{ {L"abcdefg1", L"abcdefg1"} , {L"abcdefg2", L"abcdefg2"} , {L"abcdefg3", L"abcdefg3"} , {L"abcdefg4", L"abcdefg5"} , {L"abcdefg5", L"abcdefg5"} };
            dddllbuff* dllbuff = DDDLL_TO_BUFF(T, src);
            T dst = DDDLL_FROM_BUFF(T, dllbuff);
            DDASSERT(src == dst);
        }

        {
            using T = std::unordered_map<std::wstring, std::wstring>;
            T src{ {L"abcdefg1", L"abcdefg1"} , {L"abcdefg2", L"abcdefg2"} , {L"abcdefg3", L"abcdefg3"} , {L"abcdefg4", L"abcdefg5"} , {L"abcdefg5", L"abcdefg5"} };
            dddllbuff* dllbuff = DDDLL_TO_BUFF(T, src);
            T dst = DDDLL_FROM_BUFF(T, dllbuff);
            DDASSERT(src == dst);
        }
    }
}

} // namespace NSP_DD
