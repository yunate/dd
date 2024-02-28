#ifndef ddbase_macro_ddfor_hpp_
#define ddbase_macro_ddfor_hpp_
#include "ddbase/macro/ddfor_1_.hpp"
#include "ddbase/macro/ddfor_2_.hpp"
#include "ddbase/macro/ddfor_3_.hpp"

#define __DDFOR_CAT(a, b) a ## b
#define _DDFOR_CAT(a, b) __DDFOR_CAT(a, b)

// for example:
// #define OPT(idx) int a##idx;
// DDFOR_1(OPT, 10)

// N 最大值为99, 循环范围[0, N], N取值100时候不循环
#define  DDFOR_1(opt, N) _DDFOR_CAT(_DDFOR_1_, N)(opt, N)
#define  DDFOR_2(opt, N) _DDFOR_CAT(_DDFOR_2_, N)(opt, N)
#define  DDFOR_3(opt, N) _DDFOR_CAT(_DDFOR_3_, N)(opt, N)

#endif // ddbase_macro_ddfor_hpp_