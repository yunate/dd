#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/iocp/ddiocp.h"

#include <iostream>

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
    char buff[255];
    DWORD readed = 0;

    handle = ::CreateNamedPipeA("\\\\.\\pipe\\test", PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 100, 10240, 10240, 0, NULL);
    if (!::ConnectNamedPipe(handle, &connect_ov)) {
        if (::GetLastError() != ERROR_IO_PENDING) {
            DDASSERT(false);
        }
    }

    auto pipe_callback = std::function<void(const ddiocp_item& item)> ([&](const ddiocp_item& item) {
        if (item.overlapped == &connect_ov) {
            std::cout << "pipeline connect!!!" << std::endl;
            (void)::ReadFile(handle, buff, 255, &readed, &read_ov);
        } else if (item.overlapped == &read_ov) {
            std::cout << "read:" << buff << std::endl;
            (void)::ReadFile(handle, buff, 255, &readed, &read_ov);
            if (std::string(buff) == "close iocp") {
                iocp->notify_close();
            }
        }
    });

    if (!iocp->watch(handle)) {
        DDASSERT(false);
    }

    while (true) {
        ddiocp_item item;
        ddiocp_notify_type type = iocp->wait(item);
        if (type == ddiocp_notify_type::closed) {
            break;
        } else if (type == ddiocp_notify_type::complete) {
            pipe_callback(item);
        }
        continue;
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
