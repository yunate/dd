#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"
#include "ddbase/iocp/ddiocp_io_dispatch.h"

namespace NSP_DD {
void ddiocp_io_dispatch_base::ddio_impl::run(ddiocp_with_dispatcher* iocp,
    const std::shared_ptr<ddiiocp_dispatch>& dispatch,
    const ddio_opt& opt, ddexpire expire)
{
    if (!iocp->is_run_in_iocp_thread()) {
        std::weak_ptr<ddiiocp_dispatch> wp = dispatch;
        std::function<void(bool successful, s32 byte, bool is_end)> call_back = opt.callback;
        iocp->push_task([this, wp, iocp, opt, expire, call_back]() {
            auto sp = wp.lock();
            if (sp == nullptr) {
                dderror_code::set_last_error(dderror_code::object_released);
                call_back(false, 0, false);
            } else {
                run(iocp, sp, opt, expire);
            }
        });

        return;
    }

    if (!set_ctx(opt, dispatch.get(), expire)) {
        return;
    }

    continue_run(false);
}

void ddiocp_io_dispatch_base::ddio_impl::continue_run(bool run_in_async_call)
{
    if (m_opt.async_caller != nullptr && !run_in_async_call) {
        auto wp = m_dispatch->get_iocp()->get_weaker(m_dispatch);
        std::function<void(bool successful, s32 byte, bool is_end)> call_back = m_opt.callback;
        m_opt.async_caller([this, wp, call_back]() {
            auto sp = wp.lock();
            if (sp == nullptr) {
                dderror_code::set_last_error(dderror_code::object_released);
                call_back(false, 0, false);
            } else {
                continue_run(true);
            }
        });
        return;
    }

    // 需要先设置超时, 否则在opt执行后在设置的话就要取消这次的执行
    if (m_expire != ddexpire::never) {
        u64 now = ddtime::now_ms();
        if (m_expire.epoch < now) {
            dderror_code::set_last_error(dderror_code::time_out);
            on_opt(false, 0);
            return;
        }

        m_dispatch->set_expire(&m_ov, m_expire);
    }

    if (!m_opt.exec(m_dispatch, &m_ov)) {
        m_dispatch->set_expire(&m_ov);
        if (run_in_async_call) {
            auto iocp = m_dispatch->get_iocp();
            DDASSERT_FMTW(iocp != nullptr, L"dispatch's life cycle must > iocp's life cycle.");
            auto wp = iocp->get_weaker(m_dispatch);
            std::function<void(bool successful, s32 byte, bool is_end)> call_back = m_opt.callback;
            iocp->push_task([this, wp, call_back]() {
                auto sp = wp.lock();
                if (sp == nullptr) {
                    call_back(false, 0, false);
                } else {
                    on_opt(false, 0);
                }
            });
        } else {
            on_opt(false, 0);
        }
    }
}

bool ddiocp_io_dispatch_base::ddio_impl::on_iocp_complete(const ddiocp_item& item)
{
    if (item.overlapped == &m_ov) {
        if (item.has_error) {
            dderror_code::set_last_error(item.error_code);
        }
        on_opt(!item.has_error, item.transferred_number);
        return true;
    }

    return false;
}

bool ddiocp_io_dispatch_base::ddio_impl::set_ctx(const ddio_opt& opt, ddiiocp_dispatch* dispatch, ddexpire expire)
{
    DDASSERT_FMTW(opt.exec != nullptr, L"opt.exec cannot be nullptr.");
    DDASSERT_FMTW(opt.callback != nullptr, L"opt.callback cannot be nullptr.");

    if (m_opt.callback != nullptr) {
        DDASSERT_FMTW(false, L"the next-opt (maybe write/read) must wait for the compleation of pre-call.");
        opt.callback(false, 0, false);
        return false;
    }

    m_opt = opt;
    m_dispatch = dispatch;
    m_expire = expire;
    return true;
}

void ddiocp_io_dispatch_base::ddio_impl::reset_ctx()
{
    m_opt.exec = nullptr;
    m_opt.is_end = nullptr;
    m_opt.callback = nullptr;
    m_opt.async_caller = nullptr;
    m_dispatch = nullptr;
    m_expire = ddexpire::never;
}

void ddiocp_io_dispatch_base::ddio_impl::on_opt(bool successful, s32 byte)
{
    DDASSERT(m_opt.callback != nullptr);
    bool is_end = (m_opt.is_end != nullptr) ? (m_opt.is_end()) : (true);
    if (is_end || !successful) {
        // 如果不先将m_opt.callback设置为nullptr, 然后在callback, 在callback中的请求会被拒绝
        std::function<void(bool successful, s32 byte, bool is_end)> callback = std::move(m_opt.callback);
        reset_ctx();
        callback(successful, byte, true);
        return;
    }

    m_opt.callback(true, byte, false);
    continue_run(false);
}

bool ddiocp_io_dispatch_base::ddio_impl::on_timeout(OVERLAPPED* ov)
{
    if (ov != &m_ov) {
        return false;
    }

    dderror_code::set_last_error(dderror_code::time_out);
    on_opt(false, 0);
    return true;
}
} // namespace NSP_DD

namespace NSP_DD {
void ddiocp_io_dispatch_base::write(const ddio_opt& opt, ddexpire expire)
{
    auto iocp = get_iocp();
    DDASSERT_FMTW(iocp != nullptr, L"The dispatch has not been watched.");
    auto sp = iocp->get_weaker(this).lock();
    DDASSERT(sp != nullptr);

    m_write_impl.run(iocp, sp, opt, expire);
}

void ddiocp_io_dispatch_base::read(const ddio_opt& opt, ddexpire expire)
{
    auto iocp = get_iocp();
    DDASSERT_FMTW(iocp != nullptr, L"The dispatch has not been watched.");
    auto sp = iocp->get_weaker(this).lock();
    DDASSERT(sp != nullptr);

    m_read_impl.run(iocp, sp, opt, expire);
}

void ddiocp_io_dispatch_base::on_timeout(OVERLAPPED* ov)
{
    dderror_code::set_last_error(dderror_code::time_out);
    if (m_read_impl.on_timeout(ov)) {
        return;
    }

    if (m_write_impl.on_timeout(ov)) {
        return;
    }
}

void ddiocp_io_dispatch_base::on_iocp_complete_v0(const ddiocp_item& item)
{
    if (m_read_impl.on_iocp_complete(item)) {
        return;
    }

    if (m_write_impl.on_iocp_complete(item)) {
        return;
    }

    on_iocp_complete_v1(item);
}
} // namespace NSP_DD

namespace NSP_DD {
void ddiocp_io_dispatch::write(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, ddexpire expire /* = ddexpire::never */)
{
    DDASSERT(buff_size > 0);
    ddio_opt opt;
    opt.exec = [buff, buff_size](ddiiocp_dispatch* dispatch, OVERLAPPED* ov) {
        DDASSERT(dispatch != nullptr);
        auto handle = dispatch->get_handle();
        if (handle == NULL) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        if (!::WriteFile(handle, buff, (DWORD)buff_size, NULL, ov) &&
            ::GetLastError() != ERROR_IO_PENDING) {
            return false;
        }
        return true;
    };
    opt.is_end = nullptr;
    opt.callback = [callback](bool successful, s32 byte, bool) {
        callback(successful, byte);
    };
    ddiocp_io_dispatch_base::write(opt, expire);
}

ddcoroutine<std::tuple<bool, s32>> ddiocp_io_dispatch::write(void* buff, s32 buff_size, ddexpire expire /* = ddexpire::never */)
{
    std::tuple<bool, s32> return_value;
    co_await ddco_async([this, &return_value, buff, buff_size, expire](const ddresume_helper& resumer) {
        write(buff, buff_size, [resumer, &return_value](bool successful, s32 byte) {
            std::get<0>(return_value) = successful;
            std::get<1>(return_value) = byte;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}

void ddiocp_io_dispatch::write(ddistream_view* stream_view,
    const std::function<void(bool successful, s32 byte, bool all_writed)>& callback,
    ddasync_caller async_caller, ddexpire expire /* = ddexpire::never */)
{
    auto buff = std::make_shared<std::vector<u8>>(1024);
    ddio_opt opt;
    opt.exec = [buff, stream_view](ddiiocp_dispatch* dispatch, OVERLAPPED* ov) {
        DDASSERT(dispatch != nullptr);
        auto handle = dispatch->get_handle();
        if (handle == NULL) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        s32 read_size = stream_view->read(buff->data(), (s32)buff->size());
        if (read_size <= 0) {
            return false;
        }
        if (!::WriteFile(handle, buff->data(), (DWORD)read_size, NULL, ov) &&
            ::GetLastError() != ERROR_IO_PENDING) {
            return false;
        }
        return true;
    };
    opt.is_end = [stream_view]() {
        return (stream_view->pos() == stream_view->size());
    };
    opt.callback = callback;
    opt.async_caller = async_caller;
    ddiocp_io_dispatch_base::write(opt, expire);
}

void ddiocp_io_dispatch::read(void* buff, s32 buff_size, const std::function<void(bool successful, s32 byte)>& callback, ddexpire expire /* = ddexpire::never */)
{
    DDASSERT(buff_size > 0);
    ddio_opt opt;
    opt.exec = [buff, buff_size](ddiiocp_dispatch* dispatch, OVERLAPPED* ov) {
        DDASSERT(dispatch != nullptr);
        auto handle = dispatch->get_handle();
        if (handle == NULL) {
            dderror_code::set_last_error(dderror_code::init_failure);
            return false;
        }

        if (!::ReadFile(handle, buff, (DWORD)buff_size, NULL, ov) &&
            ::GetLastError() != ERROR_IO_PENDING) {
            return false;
        }
        return true;
    };
    opt.is_end = nullptr;
    opt.callback = [callback](bool successful, s32 byte, bool) {
        callback(successful, byte);
    };
    ddiocp_io_dispatch_base::write(opt, expire);
}

ddcoroutine<std::tuple<bool, s32>> ddiocp_io_dispatch::read(void* buff, s32 buff_size, ddexpire expire /* = ddexpire::never */)
{
    std::tuple<bool, s32> return_value;
    co_await ddco_async([this, &return_value, buff, buff_size, expire](const ddresume_helper& resumer) {
        read(buff, buff_size, [resumer, &return_value](bool successful, s32 byte) {
            std::get<0>(return_value) = successful;
            std::get<1>(return_value) = byte;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_value;
}
} // namespace NSP_DD
