#ifndef ddbase_ddassert_h_
#define ddbase_ddassert_h_
#include "ddbase/dddef.h"

#ifdef _DEBUG
#include <assert.h>
#define DDASSERT(e) assert(e)
#include <crtdbg.h>
#define DDASSERT_FMTW(expr, format, ...) \
    (void) ((!!(expr)) || \
    (1 != ::_CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, format, __VA_ARGS__)) || \
    (_CrtDbgBreak(), 0))

#define DDASSERT_FMTA(expr, format, ...) \
    (void) ((!!(expr)) || \
    (1 != ::_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, format, __VA_ARGS__)) || \
    (_CrtDbgBreak(), 0))
#else
#define DDASSERT(x) ((void)0)
#define DDASSERT_FMTW(expr, format, ...)
#define DDASSERT_FMTA(expr, format, ...)
#endif

#ifdef _UNICODE 
#define DDASSERT_FMT    DDASSERT_FMTW
#else
#define DDASSERT_FMT    DDASSERT_FMTA
#endif

#endif // ddbase_ddassert_h_
