#ifndef ddbase_pickle_ddpickle_h_
#define ddbase_pickle_ddpickle_h_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddtraits.hpp"
#include "ddbase/ddmacro.hpp"

namespace NSP_DD {

class ddpickle
{
public:
    // 返回以buff开头的pickle的大小, 如果buff不能构成一个pickle,返回0
    // 该函数可以用来判断buff是否是一个有效的pickle
    static u32 get_next_pickle_size(const u8* buff, u32 buff_size);
    static u32 get_max_size();
public:
    ddpickle();

    // 从buff中构造pickle
    // 如果以buff开头的是一个有效的pickle, 将复制buff的内容
    // 如果以buff开头的不是一个有效的pickle, 将构造一个空的pickle
    ddpickle(const u8* buff, u32 buff_size);
    const ddbuff& get_buff();

    // 向pickle中write数据
    bool write_buff(u8* buff, u32 buff_size);

    // 向pickle中write一个pod类型的数据
    template<class T>
    inline bool write_pod(T data)
    {
        static_assert(!std::is_pointer<T>::value, "T should not be a pointer");
        static_assert(_container_traits<T> == ddcontainer_traits::pod, "T is not pod");
        u32 current_size = (u32)m_buff.size();
        resize((u32)m_buff.size() + sizeof(T));
        *((T*)(m_buff.data() + current_size)) = data;
        return true;
    }

    // 重置read的偏移以便能够重新从开头读取
    void reset_read_seek();

    // 从pickle中读取buff
    bool read_buff(u8* buff, u32 buff_size);

    // 从pickle中读取一个pod类型的数据
    template<class T>
    inline bool read_pod(T& data)
    {
        static_assert(!std::is_pointer<T>::value, "T should not be a pointer");
        static_assert(_container_traits<T> == ddcontainer_traits::pod, "T is not pod");
        if (m_read_seek + sizeof(T) > m_buff.size()) {
            return false;
        }

        data = *((T*)(m_buff.data() + m_read_seek)) ;
        m_read_seek += sizeof(T);
        return true;
    }

private:
    void resize(u32 size);

private:
    ddbuff m_buff;
    u32 m_read_seek = 0;
};

template<class T, ddcontainer_traits traits = _container_traits<T>>
class ddpickle_helper;

template<class T>
inline bool operator<<(ddpickle& pickle, const T& r)
{
    return ddpickle_helper<T>::write(pickle, r);
}

template<class T>
inline bool operator>>(ddpickle& pickle, T& r)
{
    return ddpickle_helper<T>::read(pickle, r);
}

inline bool operator<< (ddpickle& pickle, const char* r)
{
    return pickle << std::string(r);
}

inline bool operator<< (ddpickle& pickle, const wchar_t* r)
{
    return pickle << std::wstring(r);
}

template<>
class ddpickle_helper<std::string, ddcontainer_traits::none>
{
public:
    static bool write(ddpickle& pickle, const std::string& r)
    {
        if (!(pickle << (u32)r.size())) {
            return false;
        }
        return pickle.write_buff((u8*)r.data(), (u32)r.size());
    }

    static bool read(ddpickle& pickle, std::string& r)
    {
        u32 size = 0;
        if (!(pickle >> size)) {
            return false;
        }
        if (size > ddpickle::get_max_size()) {
            return false;
        }
        r.resize(size);
        return pickle.read_buff((u8*)r.data(), size);
    }
};

template<>
class ddpickle_helper<std::wstring, ddcontainer_traits::none>
{
public:
    static bool write(ddpickle& pickle, const std::wstring& r)
    {
        if (!(pickle << (u32)r.size() * 2)) {
            return false;
        }
        return pickle.write_buff((u8*)r.data(), (u32)r.size() * 2);
    }

    static bool read(ddpickle& pickle, std::wstring& r)
    {
        u32 size = 0;
        if (!(pickle >> size)) {
            return false;
        }
        if (size > ddpickle::get_max_size()) {
            return false;
        }
        r.resize(size / 2);
        return pickle.read_buff((u8*)r.data(), size);
    }
};

template<class T>
class ddpickle_helper<T, ddcontainer_traits::pod>
{
    static_assert(!std::is_pointer<T>::value, "T should not be a pointer");
public:
    static bool write(ddpickle& pickle, const T& r)
    {
        return pickle.write_pod(r);
    }

    static bool read(ddpickle& pickle, T& r)
    {
        return pickle.read_pod(r);
    }
};

template<class T>
class ddpickle_helper<T, ddcontainer_traits::array>
{
public:
    static bool write(ddpickle& pickle, const T& r)
    {
        if (!(pickle << (u32)r.size())) {
            return false;
        }

        for (const auto& it : r) {
            if (!(pickle << it)) {
                return false;
            }
        }
        return true;
    }

    static bool read(ddpickle& pickle, T& r)
    {
        u32 size = 0;
        if (!(pickle >> size)) {
            return false;
        }
        if (size > ddpickle::get_max_size()) {
            return false;
        }
        r.resize(size);
        for (auto& it : r) {
            if (!(pickle >> it)) {
                return false;
            }
        }
        return true;
    }
};

template<class T>
class ddpickle_helper<T, ddcontainer_traits::map>
{
public:
    static bool write(ddpickle& pickle, const T& r)
    {
        if (!(pickle << (u32)r.size())) {
            return false;
        }
        for (const auto& it : r) {
            if (!(pickle << it.first) || !(pickle << it.second)) {
                return false;
            }
        }
        return true;
    }

    static bool read(ddpickle& pickle, T& r)
    {
        u32 size = 0;
        if (!(pickle >> size)) {
            return false;
        }
        if (size > ddpickle::get_max_size()) {
            return false;
        }
        for (u32 i = 0; i < size; ++i) {
            T::key_type key;
            T::mapped_type val;
            if (!(pickle >> key) || !(pickle >> val)) {
                return false;
            }
            r[key] = val;
        }
        return true;
    }
};

template<class T>
class ddpickle_helper<T, ddcontainer_traits::set>
{
public:
    static bool write(ddpickle& pickle, const T& r)
    {
        if (!(pickle << (u32)r.size())) {
            return false;
        }
        for (const auto& it : r) {
            if (!(pickle << it)) {
                return false;
            }
        }
        return true;
    }

    static bool read(ddpickle& pickle, T& r)
    {
        u32 size = 0;
        if (!(pickle >> size)) {
            return false;
        }
        if (size > ddpickle::get_max_size()) {
            return false;
        }
        for (u32 i = 0; i < size; ++i) {
            T::key_type key;
            if (!(pickle >> key)) {
                return false;
            }
            r.insert(key);
        }
        return true;
    }
};
} // namespace NSP_DD

#define DDPICKLE_OPT_LL(a, idx) if (!(pickle << r.a)) {return false;}
#define DDPICKLE_OPT_RR(a, idx) if (!(pickle >> r.a)) {return false;}
#define DDPICKLE_OPT_BASE_STRUCT_TRAITS_W(a, idx) if (!ddpickle_helper<a, _container_traits<a>>::write(pickle, r)) {return false;}
#define DDPICKLE_OPT_BASE_STRUCT_TRAITS_R(a, idx) if (!ddpickle_helper<a, _container_traits<a>>::read(pickle, r)) {return false;}

// 最多20个参数
// eg: DDPICKLE_TRAITS_GEN_EX(StructName, DDEXPEND(BaseStruct1, BaseStruct2), arg1, arg2)
#define DDPICKLE_TRAITS_GEN_EX(StructName, BaseStructs, ...)                                                            \
template<>                                                                                                               \
class ddpickle_helper<StructName, _container_traits<StructName>>                                                         \
{                                                                                                                        \
public:                                                                                                                  \
    static bool write(ddpickle& pickle, const StructName& r)                                                             \
    {                                                                                                                    \
        pickle; r;                                                                                                       \
        DDEACH(DDPICKLE_OPT_BASE_STRUCT_TRAITS_W, BaseStructs)                                                         \
        DDEACH(DDPICKLE_OPT_LL, __VA_ARGS__);                                                                          \
        return true;                                                                                                     \
    }                                                                                                                    \
                                                                                                                         \
    static bool read(ddpickle& pickle, StructName& r)                                                                    \
    {                                                                                                                    \
        pickle; r;                                                                                                       \
        DDEACH(DDPICKLE_OPT_BASE_STRUCT_TRAITS_R, BaseStructs)                                                         \
        DDEACH(DDPICKLE_OPT_RR, __VA_ARGS__);                                                                          \
        return true;                                                                                                     \
    }                                                                                                                    \
};

// 最多20个参数
// eg: DDPICKLE_TRAITS_GEN_EX(StructName, arg1, arg2)
#define DDPICKLE_TRAITS_GEN(StructName, ...) DDPICKLE_TRAITS_GEN_EX(StructName, DDEXPEND(), __VA_ARGS__)
#endif // ddbase_pickle_ddpickle_h_

