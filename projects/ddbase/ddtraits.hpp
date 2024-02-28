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
constexpr ddcontainer_traits _container_traits<T, std::enable_if_t<std::is_standard_layout_v<T> && std::is_trivial_v<T>, void>> = ddcontainer_traits::pod;

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
} // namespace NSP_DD
#endif // ddbase_ddtraits_hpp_
