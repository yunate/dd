
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/ddperfermence.h"
#include "ddbase/windows/ddprocess.h"
#include "ddbase/str/ddstr.h"

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")

#include <dxgi.h>
#include <vector>
#include <iostream>

namespace NSP_DD {

void EnumerateAdapters(std::vector <DXGI_ADAPTER_DESC>& adapterDescs)
{
    IDXGIFactory* pFactory = NULL;
    if (FAILED(::CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)) || pFactory == NULL) {
        return;
    }

    UINT i = 0;
    while (true) {
        IDXGIAdapter* adapter = NULL;
        if (pFactory->EnumAdapters(i++, &adapter) == DXGI_ERROR_NOT_FOUND || adapter == NULL) {
            break;
        }

        DXGI_ADAPTER_DESC desc;
        if (adapter->GetDesc(&desc) == S_OK) {
            adapterDescs.push_back(desc);
        }
        adapter->Release();
    }

    pFactory->Release();
}

DDTEST(ddperfermence, gpu_useage)
{
    std::vector <DXGI_ADAPTER_DESC> adapterDescs;
    EnumerateAdapters(adapterDescs);
    if (adapterDescs.empty()) {
        return;
    }

    std::wstring processName = L"notepad.exe";
    ddgpu_perfermence gpuperfermence;
    ddgpu_perfermence::gpu_useage_desc gpuUseage;
    ddcpu_perfermence::cpu_useage_desc cpuUseage;

    gpuperfermence.get_gpu_useagedesc_byname(adapterDescs[0].AdapterLuid, processName, gpuUseage);
    ddcpu_perfermence::get_cpu_useage_desc_byname(processName, cpuUseage);

    while (true) {
        ::Sleep(1000);
        ddgpu_perfermence::gpu_useage_desc gpuUseage1;
        ddcpu_perfermence::cpu_useage_desc cpuUseage1;
        gpuperfermence.get_gpu_useagedesc_byname(adapterDescs[0].AdapterLuid, processName, gpuUseage1);
        ddcpu_perfermence::get_cpu_useage_desc_byname(processName, cpuUseage1);

        std::cout << ddstr::format("GPU:%f%% MEM_USAGE:%fM MEM_COMMIT_USAGE:%fM MEM_SHARE_USAGE:%fM",
            ddgpu_perfermence::gpu_useage_desc::get_usage(gpuUseage, gpuUseage1) * 100, gpuUseage1.mem_usage * 1.0 / 1024 / 1024, gpuUseage1.mem_commit_usage * 1.0 / 1024 / 1024, gpuUseage1.mem_share_usage * 1.0 / 1024 / 1024) << std::endl;

        std::cout << ddstr::format("CPU:%f%% MEM_USAGE:%fM MEM_PRIVATE:USAGE%fM IO_READ:%fKB/S IO_WRITE:%fKB/S",
            ddcpu_perfermence::cpu_useage_desc::get_cpu_usage(cpuUseage, cpuUseage1) * 100, cpuUseage1.mem_usage * 1.0 / 1024 / 1024, cpuUseage1.mem_private_usage * 1.0 / 1024 / 1024,
            ddcpu_perfermence::cpu_useage_desc::get_io_read_usage(cpuUseage, cpuUseage1) / 1024, ddcpu_perfermence::cpu_useage_desc::get_io_write_usage(cpuUseage, cpuUseage1) / 1024) << std::endl;
        std::cout << std::endl;
        gpuUseage = gpuUseage1;
        cpuUseage = cpuUseage1;
    }
}

} // namespace NSP_DD
