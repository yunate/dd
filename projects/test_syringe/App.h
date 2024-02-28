#ifndef test_syringe_App_h_
#define test_syringe_App_h_

#include "ddbase/ddmini_include.h"
using namespace NSP_DD;

class App
{
    DDNO_COPY_MOVE(App);
protected:
    App() = default;
    ~App() = default;

public:
    void on_dll_attach();
    void on_dll_deach();
};

#define APP (ddsingleton<App>::get_instance())
#endif // test_syringe_App_h_
