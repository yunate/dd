#ifndef ddbase_file_dddir_spyer_h_
#define ddbase_file_dddir_spyer_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/iocp/ddiocp_with_dispatcher.h"

#include <functional>
#include <string>
#include <vector>

namespace NSP_DD {

class dddir_spyer
{
    DDNO_COPY_MOVE(dddir_spyer);
    dddir_spyer() = default;
public:
    ~dddir_spyer();
    enum class type
    {
        error,
        added,
        removed,
        modified,
        rename,
    };
    static std::unique_ptr<dddir_spyer> create_inst(ddiocp_with_dispatcher* iocp);

    using continue_spy = std::function<bool()>;
    // continue_spy 调用这个函数来继续监视, 如果返回false, 内部会自动unspy
    using dddir_spyer_callback = std::function<void(const std::wstring& sub_path, type type, const continue_spy& spy)>;

    // 监视一个目录, 只监视其子目录不监视其本身, 比如这个目录被删除(重命名)的话不会被监视, 如果希望监视这种情况的话, 则需要监视其父目录
    bool spy(const std::wstring& dir_full_path, const dddir_spyer_callback& callback);
    void unspy(const std::wstring& dir_full_path);

private:
    class dddir_spied_item : public ddiiocp_dispatch
    {
    public:
        std::weak_ptr<dddir_spied_item> m_weak_this;
        dddir_spyer* m_spyer = nullptr;
        virtual ~dddir_spied_item();
        virtual HANDLE get_handle() override;
        bool spy(ddiocp_with_dispatcher* iocp, const std::wstring& dir_full_path, const dddir_spyer_callback& callback);
    private:
        bool continue_spy();
        void on_iocp_complete_v0(const ddiocp_item& item) override;

        std::wstring m_dir_full_path;
        HANDLE m_dir_handle = INVALID_HANDLE_VALUE;
        char m_buff[1024] = { 0 };
        OVERLAPPED m_ov = { 0 };
        dddir_spyer_callback m_callback = nullptr;
    };

    ddiocp_with_dispatcher* m_iocp = nullptr;
    std::map<std::wstring, std::shared_ptr<dddir_spied_item>> m_items;
};

} // namespace NSP_DD
#endif // ddbase_file_dddir_spyer_h_