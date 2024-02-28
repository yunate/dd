#include "ddbase/stdafx.h"
#include "ddbase/dderror_code.h"
#include "ddbase/iocp/ddiocp_io_dispatch.h"

namespace NSP_DD {
void ddiocp_io_dispatch_base::ddio_impl::run(const ddio_pred& pred, const ddio_callback& callback, ddexpire expire)
{
    DDASSERT(m_dispatch != nullptr);
    DDASSERT_FMTW(pred != nullptr, L"pred cannot be nullptr.");
    DDASSERT_FMTW(callback != nullptr, L"callback cannot be nullptr.");

    if (m_callback != nullptr) {
        DDASSERT_FMTW(false, L"the next-opt (maybe write/read) must wait for the compleation of pre-call.");
        callback(false, 0);
        return;
    }

    // this maybe call in the async_caller thread (called by the ddcroutine resume() function), so 
    // we must set the expire time before the pred() function, because the iocp may complete before 
    // m_callback setted(if the m_callback set after the pred).
    m_dispatch->set_expire(&m_ov, expire);
    m_callback = callback;

    if (!pred(m_dispatch, &m_ov)) {
        m_callback = nullptr;
        m_dispatch->set_expire(&m_ov);
        callback(false, 0);
        return;
    }
}

bool ddiocp_io_dispatch_base::ddio_impl::on_timeout(OVERLAPPED* ov)
{
    if (ov != &m_ov) {
        return false;
    }

    if (m_callback != nullptr) {
        dderror_code::set_last_error(dderror_code::time_out);
        auto cb = std::move(m_callback);
        cb(false, 0);
    }
    return true;
}

bool ddiocp_io_dispatch_base::ddio_impl::on_complete(const ddiocp_item& item)
{
    if (item.overlapped != &m_ov) {
        return false;
    }

    if (m_callback != nullptr) {
        auto cb = std::move(m_callback);
        if (item.has_error) {
            dderror_code::set_last_error(item.error_code);
            cb(false, 0);
        } else {
            cb(true, item.transferred_number);
        }
    }
    return true;
}
} // namespace NSP_DD

namespace NSP_DD {
ddcoroutine<std::tuple<bool, s32>> ddiocp_io_dispatch_base::write(const ddio_pred& pred, ddexpire expire)
{
    std::tuple<bool, s32> return_result;
    co_await ddawaitable([&](const ddresume_helper& resumer) {
        m_write_impl.run(pred, [resumer, &return_result](bool successful, s32 byte) {
            std::get<0>(return_result) = successful && byte != 0;
            std::get<1>(return_result) = byte;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_result;
}

ddcoroutine<std::tuple<bool, s32>> ddiocp_io_dispatch_base::read(const ddio_pred& pred, ddexpire expire)
{
    std::tuple<bool, s32> return_result;
    co_await ddawaitable([&](const ddresume_helper& resumer) {
        m_read_impl.run(pred, [resumer, &return_result](bool successful, s32 byte) {
            std::get<0>(return_result) = successful && byte != 0;
            std::get<1>(return_result) = byte;
            resumer.lazy_resume();
        }, expire);
    });
    co_return return_result;
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
    if (m_read_impl.on_complete(item)) {
        return;
    }

    if (m_write_impl.on_complete(item)) {
        return;
    }

    on_iocp_complete_v1(item);
}
} // namespace NSP_DD

namespace NSP_DD {
ddcoroutine<std::tuple<bool, s32>> ddiocp_io_dispatch::write(void* buff, s32 buff_size, ddexpire expire /* = ddexpire::never */)
{
    auto pred = [buff, buff_size](ddiiocp_dispatch* dispatch, OVERLAPPED* ov) {
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

    co_return co_await ddiocp_io_dispatch_base::write(pred, expire);
}

ddcoroutine<std::tuple<bool, s32>> ddiocp_io_dispatch::read(void* buff, s32 buff_size, ddexpire expire /* = ddexpire::never */)
{
    auto pred = [buff, buff_size](ddiiocp_dispatch* dispatch, OVERLAPPED* ov) {
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

    co_return co_await ddiocp_io_dispatch_base::read(pred, expire);
}
} // namespace NSP_DD
