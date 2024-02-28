#ifndef ddbase_ddtraits_hpp_
#define ddbase_ddtraits_hpp_
#include "ddbase/dddef.h"
#include <vector>
#include <list>
#include <deque>
#include <map>
//#include <hash_map>
#include <unordered_map>
#include <set>
#include <unordered_set>
//#include <hash_set>
#include <stack>
#include <queue>
#include <type_traits>
namespace NSP_DD {
enum class ddcontainer_traits
{
    none,
    pod,
    array,
    map,
    set,
    stack,
    queue
};
template<class, class pod_dummy = void>
constexpr ddcontainer_traits _container_traits = ddcontainer_traits::none;

template<class T>
constexpr ddcontainer_traits _container_traits<T, std::enable_if_t<std::is_standard_layout_v<T> && std::is_trivial_v<T> && !std::is_pointer_v<T>, void>> = ddcontainer_traits::pod;

template<class T>
constexpr ddcontainer_traits _container_traits<std::vector<T>> = ddcontainer_traits::array;
template<class T>
constexpr ddcontainer_traits _container_traits<std::list<T>> = ddcontainer_traits::array;
template<class T>
constexpr ddcontainer_traits _container_traits<std::deque<T>> = ddcontainer_traits::array;

template<class K, class V>
constexpr ddcontainer_traits _container_traits<std::map<K, V>> = ddcontainer_traits::map;
template<class K, class V>
constexpr ddcontainer_traits _container_traits<std::unordered_map<K, V>> = ddcontainer_traits::map;
//template<class K, class V>
//constexpr container_traits _container_traits<std::hash_map<K, V>> = container_traits::container_traits_map;

template<class K>
constexpr ddcontainer_traits _container_traits<std::set<K>> = ddcontainer_traits::set;
template<class K>
constexpr ddcontainer_traits _container_traits<std::unordered_set<K>> = ddcontainer_traits::set;
//template<class K, class V>
//constexpr container_traits _container_traits<std::hash_set<K, V>> = container_traits::container_traits_set;

template<class T>
constexpr ddcontainer_traits _container_traits<std::stack<T>> = ddcontainer_traits::stack;

template<class T>
constexpr ddcontainer_traits _container_traits<std::queue<T>> = ddcontainer_traits::queue;
////////////////////////////////////////////////////container_traits end ////////////////////////////////////////////////////////////////////

// true false value
template<class T>
constexpr bool ddtrue_v = std::is_same_v<T, T>;
template<class T>
constexpr bool ddfalse_v = !std::is_same_v<T, T>;

// scarecrow type
template<class T>
struct ddany_base_type_scarecrow {
    operator T() = delete;
    template<class U, class = std::enable_if_t<std::is_base_of_v<U, T>>> operator U();
};

struct ddany_type_scarecrow { template<typename T> operator T(); };

// true: T 有基类; false: T 没有有基类
template<class, class = void> struct ddhas_any_base : std::false_type {};
template<class T> struct ddhas_any_base<T, std::void_t<decltype(T{ ddany_base_type_scarecrow<T>{} })>> : std::true_type {};
template<class T> constexpr bool ddhas_any_base_v = ddhas_any_base<T>::value;

#if _HAS_CXX17
// 是否是C like 的struct: 没有基类, 没有构造函数/析构函数等, 没有私有变量
template<class T>
constexpr bool ddclike_struct_v = !ddhas_any_base_v<T> && std::is_aggregate_v<T> && !std::is_array_v<T>;
#endif
} // namespace NSP_DD
#endif // ddbase_ddtraits_hpp_
