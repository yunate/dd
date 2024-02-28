#ifndef ddbase_ddlazy_instance_h_
#define ddbase_ddlazy_instance_h_

#include "ddbase/ddassert.h"
namespace NSP_DD {

template <typename T>
class ddlazy_instance
{
    ddlazy_instance() = default;
    ~ddlazy_instance() = default;
public:
    // 调用者复制inst的生命周期
    static void set_inst(T* inst)
    {
        DDASSERT_FMTW(s_inst == nullptr, L"should not call `ddlazy_instance::set_inst` to init the lazy_instance again.");
        s_inst = inst;
    }
    static T& get_inst()
    {
        DDASSERT_FMTW(s_inst != nullptr, L"should call `ddlazy_instance::set_inst` to init the lazy_instance.");
        return *s_inst;
    }
    static bool has_register() { return (s_inst != nullptr); };

private:
    static T* s_inst;
};

template <typename T>
T* ddlazy_instance<T>::s_inst = nullptr;

} // namespace NSP_DD

#define DDLAZY_INSTANCE_REGISTER(inst) NSP_DD::ddlazy_instance<std::remove_reference<decltype(*inst)>::type>::set_inst(inst)
#define DDLAZY_INSTANCE(T) NSP_DD::ddlazy_instance<T>::get_inst()
#endif // ddbase_ddlazy_instance_h_

