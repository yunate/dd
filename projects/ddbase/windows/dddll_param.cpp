#include "ddbase/stdafx.h"
#include "ddbase/windows/dddll_param.h"

extern dddllrelease_t g_dddll_default_release_function = [](dddllbuff* buff) {
    if (buff == nullptr) {
        return;
    }

    if (buff->buff != nullptr) {
        delete buff->buff;
    }

    delete buff;
};
extern dddllcreate_t g_dddll_default_create_function = []() {
    return new dddllbuff {nullptr, 0, g_dddll_default_release_function };
};

void set_dddllrelease_function(dddllrelease_t function)
{
    g_dddll_default_release_function = function;
}
void set_dddllcreate_function(dddllcreate_t function)
{
    g_dddll_default_create_function = function;
}
