#ifndef ddbase_pickle_ddpickle_helper__hpp_
#define ddbase_pickle_ddpickle_helper__hpp_
namespace NSP_DD {
#if _HAS_CXX17
// , a1, a2 ...
#define DDPICKLE_HELPER_OPT_FOR_2(idx) , DDCAT(a, DDINC(idx))

// if (!(pickle << a ## idx)) { return false; }
#define DDPICKLE_HELPER_WRITE_OPT_FOR_3(idx)                            \
    if (!(pickle << a ## idx)) {                                        \
        return false;                                                   \
    }                                                                   \

#define DDPICKLE_HELPER_WRITE_OPT_FOR_1(idx)                            \
if constexpr (x == DDINC(idx)) {                                        \
    auto&& [ a0 DDFOR_2(DDPICKLE_HELPER_OPT_FOR_2, DDDEC(idx))] = r;    \
    DDFOR_2(DDPICKLE_HELPER_WRITE_OPT_FOR_3, idx)                       \
}

// if (!(pickle >> a ## idx)) { return false; }
#define DDPICKLE_HELPER_READ_OPT_FOR_3(idx)                             \
    if (!(pickle >> a ## idx)) {                                        \
        return false;                                                   \
    }                                                                   \

#define DDPICKLE_HELPER_READ_OPT_FOR_1(idx)                             \
if constexpr (x == DDINC(idx)) {                                        \
    auto&& [ a0 DDFOR_2(DDPICKLE_HELPER_OPT_FOR_2, DDDEC(idx))] = r;    \
    DDFOR_2(DDPICKLE_HELPER_READ_OPT_FOR_3, idx)                        \
}

template<class T, ddcontainer_traits traits = _container_traits<T>>
class ddpickle_helper
{
    static_assert(!std::is_pointer_v<T>, "T should not be a pointer, use DDPICKLE_TRAITS_GEN to define user's traits.");
    static_assert(ddclike_struct_v<T>, "T is not a c like struct, use DDPICKLE_TRAITS_GEN to define user's traits.");
public:
    static bool write(ddpickle& pickle, const T& r)
    {
        /*
        if constexpr (x == 1) {
            auto&& [a1] = object;
            if (!(pickle << a1)) {
                return false;
            }
        }
        if constexpr (x == 2) {
            auto&& [a1, a2] = object;
            if (!(pickle << a1)) {
                return false;
            }
            if (!(pickle << a2)) {
                return false;
            }
        }
        ...
        if constexpr (x == 100) { ... }
        */
        constexpr size_t x = ddget_member_count<T>();
        // if compile failure, use DDPICKLE_TRAITS_GEN to define user's traits.
        DDFOR_1(DDPICKLE_HELPER_WRITE_OPT_FOR_1, 99);
        return true;
    }

    static bool read(ddpickle& pickle, T& r)
    {
        /*
        if constexpr (x == 1) {
            auto&& [a1] = object;
            if (!(pickle >> a1)) {
                return false;
            }
        }
        if constexpr (x == 2) {
            auto&& [a1, a2] = object;
            if (!(pickle >> a1)) {
                return false;
            }
            if (!(pickle >> a2)) {
                return false;
            }
        }
        ...
        if constexpr (x == 100) { ... }
        */
        constexpr size_t x = ddget_member_count<T>();
        // if compile failure, use DDPICKLE_TRAITS_GEN to define user's traits.
        DDFOR_1(DDPICKLE_HELPER_READ_OPT_FOR_1, 99);
        return true;
    }
};
#else
template<class T, ddcontainer_traits traits = _container_traits<T>>
class ddpickle_helper
{
    static_assert(ddfalse_v<T>, "use DDPICKLE_TRAITS_GEN to define user's traits.");
};
#endif

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
#endif // ddbase_pickle_ddpickle_helper__hpp_