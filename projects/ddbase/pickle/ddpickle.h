#ifndef ddbase_pickle_ddpickle_h_
#define ddbase_pickle_ddpickle_h_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddtraits.hpp"
#include "ddbase/ddmember_count.hpp"
#include "ddbase/macro/ddmacro.hpp"

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
} // namespace NSP_DD
#include "ddbase/pickle/ddpickle_helper_.hpp"

#define DDPICKLE_OPT_W(a, idx) if (!(pickle << r.a)) {return false;}
#define DDPICKLE_OPT_R(a, idx) if (!(pickle >> r.a)) {return false;}
#define DDPICKLE_OPT_BASE_STRUCT_W(a, idx) if (!(pickle << (const a&)r)) {return false;}
#define DDPICKLE_OPT_BASE_STRUCT_R(a, idx) if (!(pickle >> (a&)r)) {return false;}

// 自定义写入(支持继承), 最多支持100个参数
// eg: DDPICKLE_TRAITS_GEN_EX(StructName, DDEXPEND(BaseStruct1, BaseStruct2), arg1, arg2)
#define DDPICKLE_TRAITS_GEN_EX(StructName, BaseStructs, ...)                                                         \
inline bool operator<<(NSP_DD::ddpickle& pickle, const StructName& r)                                                 \
{                                                                                                                    \
    pickle; r;                                                                                                       \
    DDEACH_1(DDPICKLE_OPT_BASE_STRUCT_W, BaseStructs)                                                                \
    DDEACH_1(DDPICKLE_OPT_W, __VA_ARGS__);                                                                           \
    return true;                                                                                                     \
}                                                                                                                    \
inline bool operator>>(NSP_DD::ddpickle& pickle, StructName& r)                                                       \
{                                                                                                                    \
    pickle; r;                                                                                                       \
    DDEACH_1(DDPICKLE_OPT_BASE_STRUCT_R, BaseStructs)                                                                \
    DDEACH_1(DDPICKLE_OPT_R, __VA_ARGS__);                                                                           \
    return true;                                                                                                     \
}                                                                                                                    \

// 自定义写入, 最多支持100个参数
// eg: DDPICKLE_TRAITS_GEN(StructName, arg1, arg2)
#define DDPICKLE_TRAITS_GEN(StructName, ...) DDPICKLE_TRAITS_GEN_EX(StructName, DDEXPEND(), __VA_ARGS__)
#endif // ddbase_pickle_ddpickle_h_
