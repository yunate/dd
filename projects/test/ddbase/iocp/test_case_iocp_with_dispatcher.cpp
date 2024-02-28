#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/iocp/ddiocp_io_dispatch.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddtime.h"

#include <iostream>

namespace NSP_DD {
class ddpipe_dispatch : public ddiocp_io_dispatch, public std::enable_shared_from_this<ddpipe_dispatch>
{
public:
    ddpipe_dispatch() {}
    virtual ~ddpipe_dispatch()
    {
        if (m_handle != NULL) {
            CloseHandle(m_handle);
        }
    };

    // @return 能够绑定到iocp的句柄
    virtual HANDLE get_handle() override
    {
        DDASSERT(m_handle != NULL);
        return m_handle;
    };

    void on_iocp_complete_v1(const ddiocp_item& item) override
    {
        if (item.overlapped == &m_connect_ov) {
            m_connect_callback(!item.has_error);
            m_connect_callback = nullptr;
        }
    }

    ddcoroutine<bool> wait_connect(ddiocp_with_dispatcher* iocp, const std::string& pipe_name, s32 buff_size)
    {
        bool result = false;
        co_await ddcoroutine_from([this, iocp, pipe_name, buff_size, &result](const ddresume_helper& resumer) {
            wait_connect_inner(pipe_name, iocp, buff_size, [resumer, iocp, &result](bool successful) {
                if (successful) {
                    result = true;
                } else {
                    std::cout << "connect failure!" << std::endl;
                }
                resumer.lazy_resume();
            });
        });
        co_return result;
    }


    bool connect(const std::string& pipe_name)
    {
        DDASSERT(m_handle == NULL);
        std::string pipe_full_name = std::string("\\\\.\\pipe\\") + pipe_name;
        m_handle = CreateFileA(pipe_full_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (m_handle == INVALID_HANDLE_VALUE) {
            m_handle = NULL;
            return false;
        }

        return true;
    }

private:
    void wait_connect_inner(const std::string& pipe_name, ddiocp_with_dispatcher* iocp, s32 buff_size, const std::function<void(bool successful)>& callback)
    {
        DDASSERT(m_handle == NULL);
        std::string pipe_full_name = std::string("\\\\.\\pipe\\") + pipe_name;
        m_handle = ::CreateNamedPipeA(pipe_full_name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 2,
            (DWORD)buff_size, (DWORD)buff_size, 0, NULL);
        if (m_handle == INVALID_HANDLE_VALUE) {
            m_handle = NULL;
            callback(false);
            return;
        }

        if (!iocp->watch(shared_from_this())) {
            callback(false);
            return;
        }
        if (!::ConnectNamedPipe(m_handle, &m_connect_ov) && ::GetLastError() != ERROR_IO_PENDING) {
            callback(false);
            return;
        }

        m_connect_callback = callback;
    }

    OVERLAPPED m_connect_ov{ 0 };
    std::function<void(bool successful)> m_connect_callback;
    HANDLE m_handle = NULL;
};

struct pipe_server
{
    ddcoroutine<void> run()
    {
        while (true) {
            auto read_result = co_await pipe->read(buff.data() + buff_remain_size, (s32)buff.size() - buff_remain_size);
            bool successful = std::get<0>(read_result);
            s32 read_size = std::get<1>(read_result);
            if (!successful) {
                std::cout << "read failure!" << std::endl;
                co_return;
            }

            buff_remain_size += read_size;
            u8* buff_begin = buff.data();
            u32 pickle_size = ddpickle::get_next_pickle_size(buff_begin, buff_remain_size);
            while (pickle_size != 0) {
                ddpickle pkg((u8*)buff_begin, pickle_size);
                std::string readed;
                pkg >> readed;
                if (readed == "close pipe") {
                    ddpickle close_pkg;
                    close_pkg << "close pipe";
                    auto write_result = co_await pipe->write((void*)close_pkg.get_buff().data(), (s32)close_pkg.get_buff().size(), ddexpire::form_timeout(1000));
                    if (!std::get<0>(write_result)) {
                        std::cout << "write failure!" << std::endl;
                    }
                    co_return;
                }

                buff_begin += pickle_size;
                buff_remain_size -= pickle_size;
                pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
            }

            if (buff_remain_size != 0 && buff_begin != buff.data()) {
                (void)::memmove(buff.data(), buff_begin, buff_remain_size);
            }
        }
    }

    pipe_server()
    {
        pipe = std::make_shared<ddpipe_dispatch>();
        buff.resize(255);
    }
    std::vector<u8> buff;
    s32 buff_remain_size = 0;
    ddiocp_with_dispatcher* iocp = nullptr;
    std::shared_ptr<ddpipe_dispatch> pipe;
};

struct pipe_client
{
    ddcoroutine<void> run()
    {
        while (true) {
            auto write_result = co_await pipe->write((void*)pickle.get_buff().data(), (s32)pickle.get_buff().size(), ddexpire::form_timeout(1000));
            if (!std::get<0>(write_result)) {
                std::cout << "write failure!" << std::endl;
            } else {
                ++count;
            }

            if (line == "close pipe") {
                char read_buff[255];
                auto read_result = co_await pipe->read(read_buff, 255, ddexpire::form_timeout(1000));
                if (!std::get<0>(read_result)) {
                    std::cout << "read failure!" << std::endl;
                    co_return;
                } else {
                    u32 pickle_size = ddpickle::get_next_pickle_size((u8*)read_buff, std::get<1>(read_result));
                    if (pickle_size > 0) {
                        //std::cout << "read successful!" << std::endl;
                        ddpickle pkg((u8*)read_buff, pickle_size);
                        std::string readed;
                        pkg >> readed;
                        std::cout << readed << std::endl;
                    }
                    else {
                        std::cout << "read failure!" << std::endl;
                    }
                }
                co_return;
            }

            // std::cout << "write successful!" << std::endl;
            if (count == 10000) {
                line = "close pipe";
                pickle = ddpickle();
                pickle << line;
            }
        }
    }

    s32 buff_remain_size = 0;
    ddiocp_with_dispatcher* iocp = nullptr;
    std::shared_ptr<ddpipe_dispatch> pipe;
    s32 count = 0;
    std::string line = "hello pipe";
    ddpickle pickle;

    pipe_client()
    {
        pickle << line;
        pipe = std::make_shared<ddpipe_dispatch>();
    }
};

ddcoroutine<void> co_srver(ddiocp_with_dispatcher* iocp)
{
    std::shared_ptr<pipe_server> server(new pipe_server());
    if (!co_await server->pipe->wait_connect( iocp, "test", 1024)) {
        co_return;
    }

    co_await server->run();
    iocp->notify_close();
}

DDTEST(test_case_co_ddiocp_with_dispatcher_server, 1)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    auto server = co_srver(iocp.get());
    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

ddcoroutine<void> co_client(ddiocp_with_dispatcher* iocp)
{
    std::shared_ptr<pipe_client> client(new pipe_client());
    if (!client->pipe->connect("test") || !iocp->watch(client->pipe)) {
        co_return;
    }

    co_await client->run();
    iocp->notify_close();
}

DDTEST(test_case_co_ddiocp_with_dispatcher_client, 1)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    auto client = co_client(iocp.get());
    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }

    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

DDTEST(test_case_sync_pipe_server, 1)
{
    ddtimer timer;
    std::string pipe_full_name = std::string("\\\\.\\pipe\\") + "test";
    auto handle = ::CreateNamedPipeA(pipe_full_name.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 2,
        1024, 1024, 0, NULL);

    if (handle == NULL) {
        DDASSERT(false);
        return;
    }
    
    if (!::ConnectNamedPipe(handle, NULL)) {
        DDASSERT(false);
        return;
    }

    char buff[1024];
    s32 buff_remain_size = 0;
    bool running = true;
    while (running) {
        DWORD read_size = 0;
        if (!::ReadFile(handle, (u8*)(buff + buff_remain_size), sizeof(buff) - buff_remain_size, &read_size, NULL)) {
            DDASSERT(false);
        }

        buff_remain_size += read_size;
        char* buff_begin = buff;
        u32 pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
        while (pickle_size != 0) {
            ddpickle pkg((u8*)buff_begin, pickle_size);
            std::string readed;
            pkg >> readed;
            if (readed == "close pipe") {
                ddpickle close_pkg;
                close_pkg << "close pipe";

                DWORD write_size = 0;
                if (!::WriteFile(handle, (void*)close_pkg.get_buff().data(), (s32)close_pkg.get_buff().size(), &write_size, NULL)) {
                    DDASSERT(false);
                }
                running = false;
                break;
            }

            buff_begin += pickle_size;
            buff_remain_size -= pickle_size;
            pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
        }

        if (buff_remain_size != 0 && buff_begin != buff) {
            (void)::memmove(buff, buff_begin, buff_remain_size);
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

DDTEST(test_case_sync_pipe_client, 1)
{
    ddtimer timer;
    std::string pipe_full_name = std::string("\\\\.\\pipe\\") + "test";
    auto handle = CreateFileA(pipe_full_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        DDASSERT(false);
        return;
    }
    ddpickle pkg;
    std::string hello = "hello";
    pkg << hello;

    for (int i = 0; i < 10000; ++i) {
        DWORD write_size = 0;
        if (!::WriteFile(handle, (void*)pkg.get_buff().data(), (s32)pkg.get_buff().size(), &write_size, NULL)) {
            DDASSERT(false);
        }
    }

    ddpickle close_pkg;
    std::string close = "close pipe";
    close_pkg << close;
    DWORD write_size = 0;
    if (!::WriteFile(handle, (void*)close_pkg.get_buff().data(), (s32)close_pkg.get_buff().size(), &write_size, NULL)) {
        DDASSERT(false);
    }

    DWORD read_size = 0;
    char buff[1024];
    if (!::ReadFile(handle, buff, sizeof(buff), &read_size, NULL)) {
        DDASSERT(false);
    }

    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}
} // namespace NSP_DD
