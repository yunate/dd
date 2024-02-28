
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include <iostream>
#include <thread>
#include <windows.h>
#include "ddbase/ipc/ddshared_memory_ipc.h"
#include "ddbase/thread/ddtask_thread.h"

namespace NSP_DD {
DDTEST(test_event, event1)
{
    HANDLE event1 = ::CreateEvent(NULL, false, false, L"test_1");
    if (event1 == NULL) {
        return;
    }

    (void)::SetEvent(event1);
    (void)::SetEvent(event1);

    int count = 0;
    while (WAIT_OBJECT_0 == ::WaitForSingleObject(event1, INFINITE))
    {
        ++count;
    }
}

DDTEST(simplex_sm_ipc, client)
{
    ddtask_thread_pool thread_manager(MAX_U32);
    ddshared_memory_ipc_client client;
    if (!client.create(L"simplex_sm_ipc", 1024 * 1024)) {
        return;
    }

    u32 send_count = 0;
    u32 count = 0;
    while (count < 100)
    {
        thread_manager.push_task([&client, count, &send_count]() {
            std::string val = std::to_string(count);
            (void)client.send(val.c_str(), (u32)val.length());
            ++send_count;
            return true;
        });
        ++count;
        ::Sleep(10);
    }

    while (send_count != count)
    {
        ::Sleep(10);
    }
}

DDTEST(simplex_sm_ipc, server)
{
    ddtask_thread_pool thread_manager(MAX_U32);
    ddshared_memory_ipc_server server;
    if (!server.create(L"simplex_sm_ipc", 1024 * 1024)) {
        return;
    }

    ddbuff buff;
    while (server.recv(buff))
    {
        std::string str((char*)buff.data(), buff.size());
        std::cout << str << std::endl;
    }
}
} // namespace NSP_DD
