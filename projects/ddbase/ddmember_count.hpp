
#ifndef ddbase_ddmember_count_hpp_
#define ddbase_ddmember_count_hpp_
#include "ddbase/dddef.h"

namespace NSP_DD {
struct ddmember_count_any_type
{
    template <typename T>
    operator T();
};

template <typename T, typename = void, typename ...Ts>
struct ddmember_count {
    constexpr static size_t value = sizeof...(Ts) - 1;
};

template <typename T, typename ...Ts>
struct ddmember_count < T, std::void_t<decltype(T{ Ts{}... }) > , Ts... > {
    constexpr static size_t value = ddmember_count<T, void, Ts..., ddmember_count_any_type>::value;
};
} // namespace NSP_DD
#endif // ddbase_ddmember_count_hpp_


