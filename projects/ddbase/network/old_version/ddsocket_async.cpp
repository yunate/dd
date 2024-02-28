#include "ddbase/stdafx.h"
#include "ddbase/network/ddsocket_async.h"
#include "ddbase/network/ddnetwork_async_caller.hpp"
#include "ddbase/dderror_code.h"
#include "ddbase/ddtimer.h"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <Ws2ipdef.h>
#include <ip2string.h>

namespace NSP_DD {
const u32 g_ddsocket_async_send_stream_buff_size = 1536;

void ddsocket_async::send(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout)
{
    DDASSERT(buff != nullptr);
    DDASSERT(buff_size != 0);
    do {
        ddsocket_async_callback wrappered_callback = [this, callback](bool successful, s32 sended) {
            if (callback != nullptr) {
                callback(successful, sended);
            }

            do_pending_send();
        };

        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (m_send_callback != nullptr || !m_pending_sends.empty()) {
            pending_context* ctx = new(std::nothrow)pending_context();
            if (ctx == nullptr) {
                break;
            }

            ctx->buff = buff;
            ctx->buff_size = buff_size;
            ctx->experid_timestamp = ddtime::get_experid_timestamp(timeout);
            ctx->callback = wrappered_callback;
            m_pending_sends.push_back(ctx);
            return;
        }

        m_send_callback = wrappered_callback;
        send_inner(buff, buff_size, ddtime::get_experid_timestamp(timeout));
        return;
    } while (0);

    run_in_next_iocp_loop([callback]() {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        callback(false, 0);
    });
}

void ddsocket_async::send_stream(ddistream* stream, const ddsocket_async_callback1& callback, u64 timeout)
{
    DDASSERT(stream != nullptr);
    DDASSERT(stream->size() != 0);
    do {
        u8* buff = new(std::nothrow) u8[g_ddsocket_async_send_stream_buff_size];
        if (buff == nullptr) {
            break;
        }

        u64 experid_timestamp = ddtime::get_experid_timestamp(timeout);
        ddsocket_async_callback wrappered_callback = [this, callback, buff, stream, experid_timestamp](bool successful, s32 sended) {
            bool need_continue_send_stream = true;
            bool all_sended = stream->pos() == stream->size();
            if (all_sended) {
                need_continue_send_stream = false;
            }

            if (callback != nullptr) {
                // if send successful and the sended count = 0, do not call the callback
                if (!(successful && sended == 0)) {
                    if (!callback(successful, all_sended, stream->pos())) {
                        need_continue_send_stream = false;
                    }
                }
            }

            if (need_continue_send_stream) {
                auto weak = get_iocp()->get_weaker(this);
                DDNETWORK_ASYNC_CALL([weak, this, stream, buff, experid_timestamp]() {
                    auto it = weak.lock();
                    if (it != nullptr) {
                        s32 readed = stream->read(buff, g_ddsocket_async_send_stream_buff_size);
                        send_inner(buff, readed, experid_timestamp);
                    }
                });
                return;
            }

            delete buff;
            do_pending_send();
        };

        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        if (m_send_callback != nullptr || !m_pending_sends.empty()) {
            pending_context* ctx = new(std::nothrow)pending_context();
            if (ctx == nullptr) {
                break;
            }

            ctx->buff_size = 0;
            ctx->callback = wrappered_callback;
            m_pending_sends.push_back(ctx);
        } else {
            m_send_callback = wrappered_callback;
            m_send_callback(true, 0);
        }

        return;
    } while (0);

    run_in_next_iocp_loop([callback]() {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        callback(false, false, 0);
    });
}

void ddsocket_async::send_inner(void* buff, s32 buff_size, u64 experid_timestamp)
{
    do {
        if (buff == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            break;
        }

        if (m_socket == INVALID_SOCKET) {
            dderror_code::set_last_error(dderror_code::init_failure);
            break;
        }

        u64 timeout = INFINITE;
        if (experid_timestamp != INFINITE) {
            u64 now = ddtime::now_ms();
            if (experid_timestamp < now) {
                dderror_code::set_last_error(dderror_code::time_out);
                break;
            }
            timeout = experid_timestamp - now;
        }

        if (!set_timeout(&m_send_ov, timeout)) {
            break;
        }

        if (!::WriteFile((HANDLE)m_socket, buff, (DWORD)buff_size, NULL, &m_send_ov)) {
            if (::GetLastError() != ERROR_IO_PENDING) {
                remove_timeout(&m_send_ov);
                break;
            }
        }
        return;
    } while (0);

    DWORD last_error = dderror_code::get_last_error();
    run_in_next_iocp_loop([this, last_error]() {
        // 能够运行到这儿说明this没有被释放
        dderror_code::set_last_error(last_error);
        ddiocp_item item;
        item.has_error = true;
        item.error_code = last_error;
        item.overlapped = &m_send_ov;
        on_iocp_complete(item);
    });
}

void ddsocket_async::do_pending_send()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_send_callback = nullptr;
    if (!m_pending_sends.empty()) {
        pending_context* ctx = m_pending_sends.front();
        m_send_callback = ctx->callback;
        if (ctx->buff_size == 0) {
            run_in_next_iocp_loop([this]() {
                m_send_callback(true, 0);
            });
        } else {
            send_inner(ctx->buff, ctx->buff_size, ctx->experid_timestamp);
        }
        delete ctx;
        m_pending_sends.pop_front();
    }
}

void ddsocket_async::recv_inner(void* buff, s32 buff_size, u64 experid_timestamp)
{
    do {
        if (buff == nullptr) {
            dderror_code::set_last_error(dderror_code::param_nullptr);
            break;
        }

        if (m_socket == INVALID_SOCKET) {
            dderror_code::set_last_error(dderror_code::init_failure);
            break;
        }

        u64 timeout = INFINITE;
        if (experid_timestamp != INFINITE) {
            u64 now = ddtime::now_ms();
            if (experid_timestamp < now) {
                dderror_code::set_last_error(dderror_code::time_out);
                break;
            }
            timeout = experid_timestamp - now;
        }

        if (!set_timeout(&m_recv_ov, timeout)) {
            break;
        }

        if (!::ReadFile((HANDLE)m_socket, buff, (DWORD)buff_size, NULL, &m_recv_ov)) {
            if (::GetLastError() != ERROR_IO_PENDING) {
                remove_timeout(&m_recv_ov);
                break;
            }
        }

        return;
    } while (0);

    DWORD last_error = dderror_code::get_last_error();
    run_in_next_iocp_loop([this, last_error]() {
        // 能够运行到这儿说明this没有被释放
        ddiocp_item item;
        item.has_error = true;
        item.error_code = last_error;
        dderror_code::set_last_error(last_error);
        item.overlapped = &m_recv_ov;
        on_iocp_complete(item);
    });
}

void ddsocket_async::recv(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout)
{
    ddsocket_async_callback wrappered_callback = [this, callback](bool successful, s32 sended) {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        ddsocket_async_callback holder = std::move(m_recv_callback);

        if (callback != nullptr) {
            callback(successful, sended);
        }
    };

    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (m_recv_callback != nullptr) {
        DDASSERT_FMTW(false, L"There can only be one recv() is running at a time");
        return;
    }

    m_recv_callback = wrappered_callback;
    recv_inner(buff, buff_size, ddtime::get_experid_timestamp(timeout));
    return;
}

HANDLE ddsocket_async::get_handle()
{
    return (HANDLE)m_socket;
}

void ddsocket_async::on_iocp_complete(const ddiocp_item& item)
{
    ddsocket_async_callback callback;
    if (item.overlapped == &m_recv_ov && m_recv_callback != nullptr) {
        callback = m_recv_callback;
    } else if (item.overlapped == &m_send_ov && m_send_callback != nullptr) {
        callback = m_send_callback;
    }

    if (callback != nullptr) {
        if (item.has_error) {
            callback(false, 0);
        } else {
            u32 transferred_number = 0;
            (void)::GetOverlappedResult((HANDLE)m_socket, item.overlapped, (LPDWORD)(&transferred_number), true);
            callback(transferred_number != 0, transferred_number);
        }
    }
}

bool ddsocket_async::test_socket()
{
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    if (!::WriteFile((HANDLE)m_socket, NULL, 0, NULL, &m_send_ov)) {
        if (::GetLastError() != ERROR_IO_PENDING) {
            return false;
        }
    }
    return true;
}
} // namespace NSP_DD

