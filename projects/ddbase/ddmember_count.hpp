#ifndef ddbase_ddmember_count_hpp_
#define ddbase_ddmember_count_hpp_
#include "ddbase/dddef.h"
#include "ddbase/ddtraits.hpp"
#include <type_traits>

namespace NSP_DD {

#if _HAS_CXX17
template <typename T, typename = void, typename... Ts>
struct ddconstructable_t : std::false_type {};

template <typename T, typename... Ts>
struct ddconstructable_t<T, std::void_t<decltype(T{ Ts{}... })> , Ts...> : std::true_type {};

template <typename T, typename... Ts>
constexpr bool ddconstructable_v = ddconstructable_t<T, void, Ts...>::value;

template <typename T, typename ...Ts>
constexpr size_t ddget_member_count()
{
    if constexpr (sizeof...(Ts) > 100) {
        static_assert(false, "The number of structural members is greater than 100, or T supports initializing lists, such as std::map which can accept any number of parameters.");
        return 100;
    }

    if constexpr (ddconstructable_v<T, Ts...>) {
        return ddget_member_count<T, ddany_type_scarecrow, Ts...>();
    } else {
        return sizeof...(Ts) - 1;
    }
}
template <typename T>
constexpr size_t ddmember_count_v = ddget_member_count<T>();
#else
template <typename T, size_t count, typename = void, typename ...Ts>
struct ddmember_count {
    constexpr static size_t value = sizeof...(Ts) - 1;
};

template <typename T, typename ...Ts>
struct ddmember_count<T, 100, void, Ts...> {
    static_assert(ddfalse_v<T>, "The number of structural members is greater than 100, or T supports initializing lists, such as std::map which can accept any number of parameters.");
    constexpr static size_t value = 100;
};

// 偏特化
template <typename T, size_t count, typename ...Ts>
struct ddmember_count<T, count, std::void_t<decltype(T{ Ts{}... })>, Ts...> {
    // 每次添加一个 ddany_type_scarecrow, 直到 T { ddany_type_scarecrow{}, ddany_type_scarecrow{}, ... } 匹配失败
    constexpr static size_t value = ddmember_count<T, count + 1, void, ddany_type_scarecrow, Ts...>::value;
};

template <typename T>
constexpr size_t ddmember_count_v = ddmember_count<T, 0>::value;
#endif
} // namespace NSP_DD
#endif // ddbase_ddmember_count_hpp_


