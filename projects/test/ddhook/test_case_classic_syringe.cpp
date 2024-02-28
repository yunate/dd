#include "test/stdafx.h"

#include "ddbase/ddtest_case_factory.h"
#include "ddhook/ddsyringe.h"

namespace NSP_DD {

DDTEST(test_classic_syringe, test_classic_syringe)
{
    auto process = std::make_shared<ddprocess>();
    process->init(L"test.exe");
#ifdef _WIN64
    ddclassic_syringe syringe(process, L"F:\\My\\dd\\bin\\Debug_x64\\test_syringe.dll");
#else
    ddclassic_syringe syringe(process, L"F:\\My\\dd\\bin\\Debug_Win32\\test_syringe.dll");
#endif

    if (!syringe.inject_dll()) {
        DDLOGW(WARNING, L"syringe.inject_dll faliure");
    }
    ::Sleep(1000 * 10);
    syringe.uninject_dll();
}

DDTEST(test_apc_syringe, test_apc_syringe)
{
    auto process = std::make_shared<ddprocess>();
    process->init(L"test.exe");
#ifdef _WIN64
    ddapc_syringe syringe(process, L"F:\\My\\dd\\bin\\Debug_x64\\test_syringe.dll");
#else
    ddapc_syringe syringe(process, L"F:\\My\\dd\\bin\\Debug_Win32\\test_syringe.dll");
#endif

    if (!syringe.inject_dll(false)) {
        DDLOGW(WARNING, L"syringe.inject_dll faliure");
    }
    ::Sleep(1000 * 10);
    syringe.uninject_dll();
}

DDTEST(test_classic_syringeex, test_classic_syringeex)
{
    auto process = std::make_shared<ddprocess>();
    process->init(L"test.exe");
#ifdef _WIN64
    ddclassic_syringeex syringe(process, L"F:\\My\\dd\\bin\\Debug_x64\\test_syringe.dll");
#else
    ddclassic_syringeex syringe(process, L"F:\\My\\dd\\bin\\Debug_Win32\\test_syringe.dll");
#endif

    if (!syringe.inject_dll()) {
        DDLOGW(WARNING, L"syringe.inject_dll faliure");
    }
    ::Sleep(1000 * 10);
    syringe.uninject_dll();
}
} // namespace NSP_DD
