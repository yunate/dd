#include "test_syringe/stdafx.h"
#include "test_syringe/App.h"
#include <windows.h>

#pragma comment(lib, "ddbase.lib")
#pragma comment(lib, "ddwin.lib")
#pragma comment(lib, "ddhook.lib")

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    (hModule);
    (lpReserved);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        APP.on_dll_attach();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        APP.on_dll_deach();
        break;
    }
    return TRUE;
}