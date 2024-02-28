#ifndef ddbase_macro_ddlogic_hpp_
#define ddbase_macro_ddlogic_hpp_

#define __DDLOGIC_CAT(a, b) a ## b
#define _DDLOGIC_CAT(a, b) __DDLOGIC_CAT(a, b)

// true / false
#define DDTRUE 1
#define DDFALSE 0

// or
#define _DDOR_00 DDFALSE
#define _DDOR_01 DDTRUE
#define _DDOR_10 DDTRUE
#define _DDOR_11 DDTRUE
#define DDOR(A, B) _DDLOGIC_CAT(_DDOR_, _DDLOGIC_CAT(A, B))

// and
#define _DDAND_00 DDFALSE
#define _DDAND_01 DDFALSE
#define _DDAND_10 DDFALSE
#define _DDAND_11 DDTRUE
#define DDAND(A, B) _DDLOGIC_CAT(_DDAND_, _DDLOGIC_CAT(A, B))

// not
#define _DDNOT_0 1
#define _DDNOT_1 0
#define DDNOT(N) _DDLOGIC_CAT(_DDNOT_, N)

// if
#define _DDIF_1(THEN, ELSE) THEN
#define _DDIF_0(THEN, ELSE) ELSE
#define DDIF(B, THEN, ELSE) _DDLOGIC_CAT(_DDIF_, B)(THEN, ELSE)
#endif // ddbase_macro_ddlogic_hpp_
