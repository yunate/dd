#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/iocp/ddiocp_with_dispatcher.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/pickle/ddpickle.h"
#include "ddbase/ddtimer.h"

#include <iostream>

namespace NSP_DD {
class ddpipe_dispatch : public ddiocp_io_dispatch
{
public:
    ddpipe_dispatch() {}
    virtual ~ddpipe_dispatch()
    {
        if (get_iocp() != nullptr) {
            get_iocp()->unwatch(this);
        }
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

    void on_iocp_complete_v2(const ddiocp_item& item) override
    {
        if (item.overlapped == &m_connect_ov) {
            m_connect_callback(!item.has_error);
        }
    }

    void wait_connect(const std::string& pipe_name, ddiocp_with_dispatcher* iocp, s32 buff_size, const std::function<void(bool successful)>& callback)
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

        if (!iocp->watch(this)) {
            callback(false);
            return;
        }
        if (!::ConnectNamedPipe(m_handle, &m_connect_ov) && ::GetLastError() != ERROR_IO_PENDING) {
            callback(false);
            return;
        }

        m_connect_callback = callback;
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
    OVERLAPPED m_connect_ov{ 0 };
    std::function<void(bool successful)> m_connect_callback;
    HANDLE m_handle = NULL;
};

struct pipe_server
{
    void read()
    {
        pipe.read(buff.data() + buff_remain_size, (s32)buff.size() - buff_remain_size, [&](bool successful, s32 read_size) {
            if (!successful) {
                std::cout << "read failure!" << std::endl;
                read();
                return;
            }

            buff_remain_size += read_size;
            u8* buff_begin = buff.data();
            u32 pickle_size = ddpickle::get_next_pickle_size(buff_begin, buff_remain_size);
            while (pickle_size != 0) {
                //std::cout << "read successful!" << std::endl;
                ddpickle pkg((u8*)buff_begin, pickle_size);
                std::string readed;
                pkg >> readed;
                std::cout << readed << std::endl;
                if (readed == "close pipe") {
                    ddpickle close_pkg;
                    close_pkg << "close pipe";
                    pipe.write((void*)close_pkg.get_buff().data(), (s32)close_pkg.get_buff().size(), [&](bool successful, s32) {
                        if (!successful) {
                            std::cout << "write failure!" << std::endl;
                        }
                        std::cout << "write successful!" << std::endl;
                        iocp->notify_close();
                    }, 1000);
                    return;
                }

                buff_begin += pickle_size;
                buff_remain_size -= pickle_size;
                pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
            }

            if (buff_remain_size != 0 && buff_begin != buff.data()) {
                (void)::memmove(buff.data(), buff_begin, buff_remain_size);
            }

            read();
        }, 1000);
    }
    pipe_server()
    {
        buff.resize(255);
    }
    std::vector<u8> buff;
    s32 buff_remain_size = 0;
    ddiocp_with_dispatcher* iocp = nullptr;
    ddpipe_dispatch pipe;
};
DDTEST(test_case_ddiocp_with_dispatcher_server, 1)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    pipe_server server;
    server.iocp = iocp.get();
    server.pipe.wait_connect("test", iocp.get(), 1024, [&iocp, &server](bool successful) {
        if (!successful) {
            std::cout << "connected failure!" << std::endl;
            iocp->notify_close();
            return;
        }
        std::cout << "connected successful!" << std::endl;
        server.read();
    });

    if (!iocp->watch(&server.pipe)) {
        return;
    }

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

struct pipe_client
{
    void write()
    {
        pipe.write((void*)pickle.get_buff().data(), (s32)pickle.get_buff().size(), [&](bool successful, s32) {
            if (!successful) {
                std::cout << "write failure!" << std::endl;
            } else {
                ++count;
            }

            if (line == "close pipe") {
                char read_buff[255];
                pipe.read(read_buff, 255, [&](bool successful, s32 byte) {
                    if (!successful) {
                        std::cout << "read failure!" << std::endl;
                    } else {
                        u32 pickle_size = ddpickle::get_next_pickle_size((u8*)read_buff, byte);
                        if (pickle_size > 0) {
                            //std::cout << "read successful!" << std::endl;
                            ddpickle pkg((u8*)read_buff, pickle_size);
                            std::string readed;
                            pkg >> readed;
                            std::cout << readed << std::endl;
                        } else {
                            std::cout << "read failure!" << std::endl;
                        }
                    }
                    iocp->notify_close();
                }, 1000);
                return;
            }
            // std::cout << "write successful!" << std::endl;
            if (count == 10000) {
                line = "close pipe";
                pickle = ddpickle();
                pickle << line;
            }
            write();
        }, 1000);
    }

    s32 buff_remain_size = 0;
    ddiocp_with_dispatcher* iocp = nullptr;
    ddpipe_dispatch pipe;
    s32 count = 0;
    std::string line = "hello pipe";
    ddpickle pickle;

    pipe_client()
    {
        pickle << line;
    }
};

DDTEST(test_case_ddiocp_with_dispatcher_client, 1)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    pipe_client client;
    client.iocp = iocp.get();
    if (!client.pipe.connect("test")) {
        std::cout << "connect failure!" << std::endl;
        iocp->notify_close();
        return;
    }

    if (!iocp->watch(&client.pipe)) {
        std::cout << "watch failure!" << std::endl;
        return;
    }

    std::string send = "hello pipe";
    client.write();
    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

ddcoroutine<bool> co_wait_connect(ddpipe_dispatch& pipe, ddiocp_with_dispatcher* iocp, const std::string& pipe_name, s32 buff_size)
{
    bool result = false;
    co_await ddcoroutine_from([&pipe, iocp, pipe_name, buff_size, &result](const std::function<void()>& resumer) {
        pipe.wait_connect(pipe_name, iocp, buff_size, [resumer, &pipe, iocp, &result](bool successful) {
            if (successful) {
                std::cout << "connect successful!" << std::endl;
                result = true;
            } else {
                std::cout << "connect failure!" << std::endl;
            }
            resumer();
        });
    });
    co_return result;
}

ddcoroutine<s32> co_read(ddpipe_dispatch& pipe, void* buff, s32 buff_size, u64 timeout)
{
    s32 result = 0;
    co_await ddcoroutine_from([&pipe, buff, buff_size, timeout, &result](const std::function<void()>& resumer) {
        pipe.read(buff, buff_size, [resumer, &result](bool , s32 byte) {
            result = byte;
            resumer();
        }, timeout);
    });
    co_return result;
}

ddcoroutine<s32> co_write(ddpipe_dispatch& pipe, void* buff, s32 buff_size, u64 timeout)
{
    s32 result = 0;
    co_await ddcoroutine_from([&pipe, buff, buff_size, timeout, &result](const std::function<void()>& resumer) {
        pipe.write(buff, buff_size, [resumer, &result](bool , s32 byte) {
            result = byte;
            resumer();
        }, timeout);
    });
    co_return result;
}

ddcoroutine<void> co_srver(ddiocp_with_dispatcher* iocp)
{
    ddexec_guard guard([iocp]() {
        iocp->notify_close();
    });

    ddpipe_dispatch pipe;
    if (!co_await co_wait_connect(pipe, iocp, "test", 1024)) {
        co_return;
    }
    char buff[255];
    s32 buff_remain_size = 0;
    while (true) {
        s32 read_size = co_await co_read(pipe, (u8*)(buff + buff_remain_size), 255 - buff_remain_size, 1000);
        if (read_size == 0) {
            std::cout << "read failure!" << std::endl;
            continue;
        }

        buff_remain_size += read_size;
        char* buff_begin = buff;
        u32 pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
        while (pickle_size != 0) {
            //
            //std::cout << "read successful!" << std::endl;
            ddpickle pkg((u8*)buff_begin, pickle_size);
            std::string readed;
            pkg >> readed;
            std::cout << readed << std::endl;
            if (readed == "close pipe") {
                ddpickle close_pkg;
                close_pkg << "close pipe";
                s32 writed = co_await co_write(pipe, (void*)close_pkg.get_buff().data(), (s32)close_pkg.get_buff().size(), 1000);
                if (writed == 0) {
                    std::cout << "write failure!" << std::endl;
                } else {
                    //std::cout << "write successful!" << std::endl;
                }
                co_return;
            }

            buff_begin += pickle_size;
            buff_remain_size -= pickle_size;
            pickle_size = ddpickle::get_next_pickle_size((u8*)buff_begin, buff_remain_size);
        }

        if (buff_remain_size != 0 && buff_begin != buff) {
            (void)::memmove(buff, buff_begin, buff_remain_size);
        }
    }
}

DDTEST(test_case_co_ddiocp_with_dispatcher_server, 1)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    auto server = co_srver(iocp.get());
    server.run();

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }
    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

ddcoroutine<void> co_client(ddiocp_with_dispatcher* iocp)
{
    ddexec_guard guard([iocp]() {
        iocp->notify_close();
    });
    ddpipe_dispatch pipe;
    if (!pipe.connect("test") || !iocp->watch(&pipe)) {
        std::cout << "connect failure!" << std::endl;
        co_return;
    }
    ddpickle hello_pkg;
    hello_pkg << "hello pipe";

    for (int i = 0; i < 10000; ++i) {
        s32 writed = co_await co_write(pipe, (void*)hello_pkg.get_buff().data(), (s32)hello_pkg.get_buff().size(), 1000);
        if (writed == 0) {
            std::cout << "write failure!" << std::endl;
        } else {
            //std::cout << "write successful!" << std::endl;
        }
    }

    ddpickle close_pkg;
    close_pkg << "close pipe";
    s32 writed = co_await co_write(pipe, (void*)close_pkg.get_buff().data(), (s32)close_pkg.get_buff().size(), 1000);
    if (writed == 0) {
        std::cout << "write failure!" << std::endl;
    } else {
        //std::cout << "write successful!" << std::endl;
    }

    char buff[255];
    s32 read_size = co_await co_read(pipe, (u8*)(buff), 255, 1000);
    if (read_size == 0) {
        std::cout << "read failure!" << std::endl;
    } else {
        u32 pickle_size = ddpickle::get_next_pickle_size((u8*)buff, read_size);
        if (pickle_size > 0) {
            //std::cout << "read successful!" << std::endl;
            ddpickle pkg((u8*)buff, pickle_size);
            std::string readed;
            pkg >> readed;
            std::cout << readed << std::endl;
        } else {
            std::cout << "read failure!" << std::endl;
        }
    }
}

DDTEST(test_case_co_ddiocp_with_dispatcher_client, 1)
{
    ddtimer timer;
    auto iocp = ddiocp_with_dispatcher::create_instance();
    if (iocp == nullptr) {
        return;
    }

    auto client = co_client(iocp.get());
    client.run();

    while (true) {
        if (!iocp->dispatch()) {
            break;
        }
    }

    std::cout << timer.get_time_pass() / 1000000 << std::endl;
}

} // namespace NSP_DD
