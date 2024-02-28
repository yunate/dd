#ifndef ddbase_macro_ddeach_hpp_
#define ddbase_macro_ddeach_hpp_

#include "ddbase/macro/ddsize.hpp"
#include "ddbase/macro/ddinc.hpp"
#include "ddbase/macro/ddlogic.hpp"
#include "ddbase/macro/ddeach_1_.hpp"
#include "ddbase/macro/ddeach_2_.hpp"
#include "ddbase/macro/ddeach_3_.hpp"


// for each example:
// #define OPT(a, idx) int a;
// DDEACH_1(OPT, a, b, c)
#define  DDEACH_1(opt, ...) _DDEACH_CAT1(_DDEACH_1_, DDARGS_SIZE(__VA_ARGS__)(opt, 0, __VA_ARGS__))
#define  DDEACH_2(opt, ...) _DDEACH_CAT2(_DDEACH_2_, DDARGS_SIZE(__VA_ARGS__)(opt, 0, __VA_ARGS__))
#define  DDEACH_3(opt, ...) _DDEACH_CAT3(_DDEACH_3_, DDARGS_SIZE(__VA_ARGS__)(opt, 0, __VA_ARGS__))
#endif // ddbase_macro_ddeach_hpp_
