#include "test/stdafx.h"
#include "ddbase/ddtraits.hpp"
#if _HAS_CXX17
struct A {
    ~A() {}
    std::string str;
    std::wstring wstr;
    std::vector<std::string> strs;
    std::map<std::string, std::string> map;
    std::unordered_map<std::string, std::string> umap;
    std::set<std::string> set;
    std::unordered_set<std::string> uset;
};

static_assert(NSP_DD::ddclike_struct_v<A>, "");

struct B {
    B() {}
    std::string str;
};

static_assert(!NSP_DD::ddclike_struct_v<B>, "B has construct function, it is not a c like struct");

struct C : A {
    std::string str;
};

static_assert(!NSP_DD::ddclike_struct_v<C>, "C has base struct, it is not a c like struct");

struct D {
    std::string str;
private:
    int x;
};

static_assert(!NSP_DD::ddclike_struct_v<D>, "D has private data, it is not a c like struct");
#endif
