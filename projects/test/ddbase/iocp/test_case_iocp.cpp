#include "test/stdafx.h"

#include "ddbase/network/ddnetwork.h"
#include "ddbase/iocp/ddiocp.h"

#include "ddbase/ddtest_case_factory.h"

#include <iostream>
//#include <WinSock2.h>
//#pragma comment(lib, "ws2_32.lib")

#include <ppl.h>


#include <WinSock2.h>
#include <mswsock.h>
#pragma comment(lib, "mswsock.lib")
namespace NSP_DD {

DDTEST(test_case_iocp, ddiocp_pipe_server)
{
    auto iocp = ddiocp::create_instance();
    if (iocp == nullptr) {
        DDASSERT(false);
    }

    HANDLE handle = NULL;
    OVERLAPPED connect_ov{ 0 };
    OVERLAPPED read_ov{ 0 };
    //char read_buff[1024] = { 0 };
    OVERLAPPED write_ov{ 0 };
    //char write_buff[1024] = { 0 };

    handle = ::CreateNamedPipeA("\\\\.\\pipe\\test", PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 100, 10240, 10240, 0, NULL);
    if (!::ConnectNamedPipe(handle, &connect_ov)) {
        if (::GetLastError() != ERROR_IO_PENDING) {
            DDASSERT(false);
        }
    }

    auto pipe_callback = std::function<void(const ddiocp_item& item)> ([&](const ddiocp_item& item) {
        if (item.overlapped == &connect_ov) {
            std::cout << "pipeline connect!!!" << std::endl;
        }

        if (handle != NULL) {
            WriteFile(handle, "hello", 6, NULL, &write_ov);
            WriteFile(handle, "hello", 6, NULL, &write_ov);
            WriteFile(handle, "hello", 6, NULL, &write_ov);
            WriteFile(handle, "hello", 6, NULL, &write_ov);
            WriteFile(handle, "hello", 6, NULL, &write_ov);
        }
        Sleep(10);

        if (handle != NULL) {
            CloseHandle(handle);
            handle = NULL;
        }
        //DWORD readed = 0;
        //BOOL result = ::GetOverlappedResult(handle, item.overlapped, &readed, FALSE);
        //if (item.overlapped == &read_ov && result && readed != 0) {
        //    std::cout << read_buff << std::endl;
        //    if (std::string(read_buff) == "close iocp") {
        //        iocp.close();
        //    }
        //}

        //// continue read;
        //(void)::ReadFile(handle, read_buff, sizeof(read_buff), &readed, &read_ov);
    });

    if (!iocp->watch(handle, (size_t)&pipe_callback)) {
        DDASSERT(false);
    }

    while (true) {
        ddiocp_item item;
        ddiocp_notify_type type = iocp->wait(item);
        if (type == ddiocp_notify_type::closed) {
            break;
        } else if (type == ddiocp_notify_type::timeout ||
            type == ddiocp_notify_type::inner_task_run) {
            continue;
        }

        (*((std::function<void(const ddiocp_item & item)>*)item.key))(item);
    }
}

DDTEST(test_case_iocp1, ddiocp_pipe_client)
{
    HANDLE hPipe = CreateFileA("\\\\.\\pipe\\test", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    std::string hello = "hello";
    DWORD written = 0;
    (void)WriteFile(hPipe, hello.c_str(), (DWORD)hello.length() + 1, &written, NULL);
    std::string close = "close iocp";
    (void)WriteFile(hPipe, close.c_str(), (DWORD)close.length() + 1, &written, NULL);

}

} // namespace NSP_DD
