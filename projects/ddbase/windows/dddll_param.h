#ifndef ddbase_windows_dddll_param_h_
#define ddbase_windows_dddll_param_h_
#include "ddbase/ddassert.h"
#include "ddbase/ddexec_guard.hpp"
#include "ddbase/ddtraits.hpp"
#include "ddbase/macro/ddmacro.hpp"

#include <windows.h>

struct dddllbuff;
typedef dddllbuff dddllstring;
typedef dddllbuff dddllwstring;
typedef dddllbuff dddllvector;
typedef dddllbuff dddllset;
typedef dddllbuff dddllmap;
typedef dddllbuff dddllunorderedset;
typedef dddllbuff dddllunorderedmap;

typedef void (*dddllrelease_t)(dddllbuff*);
typedef dddllbuff* (*dddllcreate_t)();

#pragma pack(push, 1)
struct dddllbuff
{
    void* buff;
    NSP_DD::u32 length;
    dddllrelease_t release_function;
};
#pragma pack(pop)

//////////////////////////////////////---//////////////////////////////////////
// 设置new/delete dddllbuff函数
// 在设置create函数时候，新设置的函数需要为dddllbuff.release_function赋值
void set_dddllrelease_function(dddllrelease_t);
void set_dddllcreate_function(dddllrelease_t);
extern dddllrelease_t g_dddll_default_release_function;
extern dddllcreate_t g_dddll_default_create_function;

#define DDDLL_FROM_BUFF(T, buff) dddllbuff_to<T>(buff)
#define DDDLL_TO_BUFF(T, value) to_dddllbuff<T>(value)

#define RELEASE_GUARD()                               \
NSP_DD::ddexec_guard guard([buff]() {                    \
    if (buff == nullptr) {                            \
        return;                                       \
    }                                                 \
    DDASSERT(buff->release_function != nullptr);     \
    buff->release_function(buff);                     \
});                                                   \

#define CHECK_AUTO_RELEASE(buff)                      \
DDASSERT(buff != nullptr);                           \
RELEASE_GUARD();                                      \


template<class T, NSP_DD::ddcontainer_traits traits = NSP_DD::_container_traits<T>>
class dddllbuff_helper;

template<class T>
T dddllbuff_to(dddllbuff* buff)
{
    return dddllbuff_helper<T>::dddllbuff_to(buff);
}

template<class T>
dddllbuff* to_dddllbuff(const T& src)
{
    return dddllbuff_helper<T>::to_dddllbuff(src);
}

//////////////////////////////////////string/wstring//////////////////////////////////////
template<>
class dddllbuff_helper<std::string, NSP_DD::ddcontainer_traits::none>
{
public:
    static std::string dddllbuff_to(dddllbuff* buff)
    {
        CHECK_AUTO_RELEASE(buff);
        if (buff->length == 0 || buff->length == 1) {
            return std::string();
        }

        return (char*)(buff->buff);
    }

    static dddllbuff* to_dddllbuff(const std::string& src)
    {
        dddllbuff* buff = g_dddll_default_create_function();
        if (src.empty()) {
            return buff;
        }

        buff->length = (NSP_DD::u32)(src.length() + 1);
        buff->buff = new char[buff->length];
        ::memcpy_s(buff->buff, buff->length, src.data(), buff->length);
        return buff;
    }
};

template<>
class dddllbuff_helper<std::wstring, NSP_DD::ddcontainer_traits::none>
{
public:
    static std::wstring dddllbuff_to(dddllbuff* buff)
    {
        CHECK_AUTO_RELEASE(buff);
        if (buff->length == 0 || buff->length == 2) {
            return std::wstring();
        }

        return (wchar_t*)(buff->buff);
    }

    static dddllbuff* to_dddllbuff(const std::wstring& src)
    {
        dddllbuff* buff = g_dddll_default_create_function();
        if (src.empty()) {
            return buff;
        }

        buff->length = (NSP_DD::u32)((src.length() + 1) * 2);
        buff->buff = new char[buff->length];
        ::memcpy_s(buff->buff, buff->length, src.data(), buff->length);
        return buff;
    }
};

//////////////////////////////////////pod//////////////////////////////////////
template<typename T>
class dddllbuff_helper<T, NSP_DD::ddcontainer_traits::pod>
{
public:
    static T dddllbuff_to(dddllbuff* buff)
    {
        CHECK_AUTO_RELEASE(buff);
        DDASSERT(buff->length == sizeof(T));
        DDASSERT(buff->length != 0);
        T returnpod{};
        ::memcpy_s(&returnpod, buff->length, buff->buff, buff->length);
        return returnpod;
    }

    static dddllbuff* to_dddllbuff(const T& src)
    {
        dddllbuff* buff = g_dddll_default_create_function();
        buff->length = sizeof(T);
        buff->buff = new char[buff->length];
        ::memcpy_s(buff->buff, buff->length, &src, buff->length);
        return buff;
    }
};

//////////////////////////////////////vector//////////////////////////////////////
template<typename T>
class dddllbuff_helper<T, NSP_DD::ddcontainer_traits::array>
{
public:
    using F = typename T::value_type;
    static std::vector<F> dddllbuff_to(dddllbuff* buff)
    {
        CHECK_AUTO_RELEASE(buff);
        std::vector<F> returnvec;
        if (buff->length == 0) {
            return returnvec;
        }

        returnvec.resize(buff->length / sizeof(dddllbuff*));
        for (size_t i = 0; i < returnvec.size(); ++i) {
            returnvec[i] = dddllbuff_helper<F>::dddllbuff_to(*((dddllbuff**)buff->buff + i));
        }
        return returnvec;
    }

    static dddllbuff* to_dddllbuff(const std::vector<F>& src)
    {
        dddllbuff* buff = g_dddll_default_create_function();
        if (src.empty()) {
            return buff;
        }

        buff->length = NSP_DD::u32(src.size() * sizeof(dddllbuff*));
        buff->buff = new char[buff->length];
        for (size_t i = 0; i < src.size(); ++i) {
            *((dddllbuff**)buff->buff + i) = dddllbuff_helper<F>::to_dddllbuff(src[i]);
        }
        return buff;
    }
};

////////////////////////////////////////set//////////////////////////////////////
template<class T>
class dddllbuff_helper<T, NSP_DD::ddcontainer_traits::set>
{
public:
    using F = typename T::key_type;
    static T dddllbuff_to(dddllbuff* buff)
    {
        CHECK_AUTO_RELEASE(buff);
        T returnset{};
        if (buff->length == 0) {
            return returnset;
        }

        for (NSP_DD::u32 i = 0; i < buff->length / sizeof(dddllbuff*); ++i) {
            returnset.insert(dddllbuff_helper<F>::dddllbuff_to(*((dddllbuff**)buff->buff + i)));
        }
        return returnset;
    }

    static dddllbuff* to_dddllbuff(const T& src)
    {
        dddllbuff* buff = g_dddll_default_create_function();
        if (src.empty()) {
            return buff;
        }

        buff->length = NSP_DD::u32(src.size() * sizeof(dddllbuff*));
        buff->buff = new char[buff->length];
        NSP_DD::u32 i = 0;
        for (auto it : src) {
            *((dddllbuff**)buff->buff + i) = dddllbuff_helper<F>::to_dddllbuff(it);
            ++i;
        }
        return buff;
    }
};

//////////////////////////////////////map//////////////////////////////////////
template<class T>
class dddllbuff_helper<T, NSP_DD::ddcontainer_traits::map>
{
public:
    using K = typename T::key_type;
    using V = typename T::mapped_type;
    static T dddllbuff_to(dddllbuff* buff)
    {
        CHECK_AUTO_RELEASE(buff);
        T returnmap{};
        if (buff->length == 0) {
            return returnmap;
        }

        for (NSP_DD::u32 i = 0; i < buff->length / sizeof(dddllbuff*) / 2; ++i) {
            
            returnmap[dddllbuff_helper<K>::dddllbuff_to(*((dddllbuff**)buff->buff + 2 * i))] =
                dddllbuff_helper<V>::dddllbuff_to(*((dddllbuff**)buff->buff + 2 * i + 1));
        }
        return returnmap;
    }

    static dddllbuff* to_dddllbuff(const T& src)
    {
        dddllbuff* buff = g_dddll_default_create_function();
        if (src.empty()) {
            return buff;
        }

        buff->length = NSP_DD::u32(src.size() * sizeof(dddllbuff*) * 2);
        buff->buff = new char[buff->length];
        NSP_DD::u32 i = 0;
        for (auto it : src) {
            *((dddllbuff**)buff->buff + 2 * i) = dddllbuff_helper<K>::to_dddllbuff(it.first);
            *((dddllbuff**)buff->buff + 2 * i + 1) = dddllbuff_helper<V>::to_dddllbuff(it.second);
            ++i;
        }
        return buff;
    }
};
#endif // ddbase_windows_dddll_param_h_

