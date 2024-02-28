#ifndef ddbase_ddstatic_initer_h_
#define ddbase_ddstatic_initer_h_

#include "ddbase/dddef.h"

#define DDSTATIC_INITER(name) static void __ddstatic_initer_proc__ ## name()
#define DDSTATIC_DEINITER(name)  static void __ddstatic_deiniter_proc__ ## name()
#define DDSTATIC_INIT(name)                         \
class ddstatic_initer ## name                       \
{                                                   \
public:                                             \
    ddstatic_initer ## name()                       \
    {                                               \
        __ddstatic_initer_proc__ ## name();         \
    }                                               \
    ~ddstatic_initer ## name()                      \
    {                                               \
        __ddstatic_deiniter_proc__ ## name();       \
    }                                               \
};                                                  \
static ddstatic_initer ## name s_ ## name ## dummy

#endif // ddbase_ddstatic_initer_h_
