#include "test_syringe/stdafx.h"
#include "test_syringe/App.h"
#include "ddhook/dd_winapi_hooker.h"

using namespace NSP_DD;
int WINAPI messageboxw_proc(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    lpText = "hooked_text";
    lpCaption = "hooked_caption";
    int rtn = -1;
    DDHOOKER_MANAGER.exec_raw("MessageBoxA", [&]() {
        rtn = ::MessageBoxA(hWnd, lpText, lpCaption, uType);
    });
    return rtn;
}

void App::on_dll_attach()
{
    HMODULE api_dll = ::LoadLibrary(L"User32.dll");
    DDHOOKER_MANAGER.regist_hooker("MessageBoxA", "MessageBoxA", api_dll, messageboxw_proc, nullptr);
    ::MessageBoxA(0, "dll attach", "title", 0);
}

void App::on_dll_deach()
{
    ::MessageBoxA(0, "dll attach", "title", 0);
    ::MessageBox(0, L"dll deach", L"title", 0);
}
