
#include "ddbase/stdafx.h"
#include "ddbase/file/dddir_spyer.h"
#include "ddbase/file/dddir.h"
#include "ddbase/file/ddpath.h"

#include <queue>
#include <memory>

#include <Windows.h>
namespace NSP_DD {
dddir_spyer::~dddir_spyer()
{
}

std::unique_ptr<dddir_spyer> dddir_spyer::create_inst(ddiocp_with_dispatcher* iocp)
{
    DDASSERT(iocp != nullptr);
    std::unique_ptr<dddir_spyer> inst(new (std::nothrow) dddir_spyer());
    if (inst == nullptr) {
        return nullptr;
    }

    inst->m_iocp = iocp;
    return inst;
}

bool dddir_spyer::spy(const std::wstring& dir_full_path, const dddir_spyer_callback& callback)
{
    if (!dddir::is_path_exist(dir_full_path) || !dddir::is_dir(dir_full_path)) {
        dderror_code::set_last_error(ERROR_FILE_NOT_FOUND);
        return false;
    }

    std::shared_ptr<dddir_spied_item> item = std::make_shared<dddir_spied_item>();
    item->m_weak_this = item;
    if (!item->spy(m_iocp, dir_full_path, callback)) {
        return false;
    }

    unspy(dir_full_path);
    m_items[dir_full_path] = item;
    item->m_spyer = this;
    return true;
}

void dddir_spyer::unspy(const std::wstring& dir_full_path)
{
    auto it = m_items.find(dir_full_path);
    if (it == m_items.end()) {
        return;
    }

    m_items.erase(it);
}
} // namespace NSP_DD

namespace NSP_DD {
HANDLE dddir_spyer::dddir_spied_item::get_handle()
{
    return m_dir_handle;
}

dddir_spyer::dddir_spied_item::~dddir_spied_item()
{
    if (m_dir_handle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_dir_handle);
        m_dir_handle = INVALID_HANDLE_VALUE;
    }
}

bool dddir_spyer::dddir_spied_item::spy(ddiocp_with_dispatcher* iocp, const std::wstring& dir_full_path, const dddir_spyer_callback& callback)
{
    DDASSERT(iocp != nullptr);
    DDASSERT(callback != nullptr);
    m_dir_full_path = dir_full_path;
    m_callback = callback;
    // m_ov.hEvent = ::CreateEvent();
    m_dir_handle = ::CreateFile(
        dir_full_path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);

    if (!iocp->watch(m_weak_this)) {
        return false;
    }

    return continue_spy();
}

bool dddir_spyer::dddir_spied_item::continue_spy()
{
    ::ZeroMemory(m_buff, sizeof(m_buff));
    if (!::ReadDirectoryChangesW(m_dir_handle,
        &m_buff,
        sizeof(m_buff),
        TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME
        | FILE_NOTIFY_CHANGE_DIR_NAME
        // | FILE_NOTIFY_CHANGE_ATTRIBUTES
        | FILE_NOTIFY_CHANGE_SIZE
        // | FILE_NOTIFY_CHANGE_LAST_WRITE
        // | FILE_NOTIFY_CHANGE_LAST_ACCESS
        // | FILE_NOTIFY_CHANGE_CREATION
        // | FILE_NOTIFY_CHANGE_SECURITY
        ,
        NULL, &m_ov, NULL)) {
        return false;
    }
    return true;
}

static dddir_spyer::type native_2_dddir_spyer_type(FILE_NOTIFY_INFORMATION* notification)
{
    dddir_spyer::type type = dddir_spyer::type::error;
    if (notification->Action == FILE_ACTION_ADDED) {
        type = dddir_spyer::type::added;
    } else if (notification->Action == FILE_ACTION_REMOVED) {
        type = dddir_spyer::type::removed;
    } else if (notification->Action == FILE_ACTION_MODIFIED) {
        type = dddir_spyer::type::modified;
    } else if (notification->Action == FILE_ACTION_RENAMED_OLD_NAME) {
        type = dddir_spyer::type::rename;
    } else if (notification->Action == FILE_ACTION_RENAMED_NEW_NAME) {
        type = dddir_spyer::type::rename;
    }
    return type;
}

void dddir_spyer::dddir_spied_item::on_iocp_complete_v0(const ddiocp_item& item)
{
    if (m_callback == nullptr) {
        return;
    }

    if (item.transferred_number == 0) {
        m_callback(L"", dddir_spyer::type::error, [this]() {
            return continue_spy();
        });
        return;
    }

    FILE_NOTIFY_INFORMATION* notification = (FILE_NOTIFY_INFORMATION*)m_buff;
    m_callback(notification->FileName, native_2_dddir_spyer_type(notification), [this]() {
        DDASSERT(m_spyer != nullptr);
        bool result = continue_spy();
        if (!result) {
            m_spyer->unspy(m_dir_full_path);
        }
        return result;
    });
}
} // namespace NSP_DD
