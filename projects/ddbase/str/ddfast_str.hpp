
#ifndef ddbase_str_ddfast_str_hpp_
#define ddbase_str_ddfast_str_hpp_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"

namespace NSP_DD {
template <class t>
class ddfast_str_basic {
public:
    using t_str = std::basic_string<t>;
    using t_size = typename t_str::size_type;
    static constexpr auto xnpos = t_str::npos;

public:
    ddfast_str_basic(const t* buff);
    ddfast_str_basic(const t_str& str);
    ddfast_str_basic(const t* buff, t_size len);
    ddfast_str_basic(const t* buff, t_size l, t_size r);

    inline const t* data() const;
    inline t_size length() const;
    inline t operator[](t_size index) const;
    inline bool operator==(const ddfast_str_basic& r) const;
    inline bool operator!=(const ddfast_str_basic& r) const;

public:
    inline t_str to_str() const;
    inline const t* c_str() const;

    void trim();
    void trim(t c);
    // 匹配出现在trims中的字符
    void trim(const std::vector<t>& trims);
    // 匹配trim
    void trim(const ddfast_str_basic& trim);

    /**
    @param [in] cmp 分割符, 为空时返回true
    */
    bool beg_with(const ddfast_str_basic& cmp) const;
    bool end_with(const ddfast_str_basic& cmp) const;

    ddfast_str_basic sub_str(t_size l, t_size len = xnpos) const;
    void to_sub_str(t_size l, t_size len = xnpos);

    /**
    @param [in] cmp 分割符, 为空时返回0
    */
    t_size find(t c) const;
    t_size find(const ddfast_str_basic& cmp) const;

    /**
    @param [in] cmp 分割符, 为空时返回-1
    */
    t_size find_last_of(t c) const;
    t_size find_last_of(const ddfast_str_basic& cmp) const;

    /**
    @param [in] cmp 分割符, 调用者保证不为空字符串
    @param [out] out 结果, 不包含空字符串
    */
    void split(const ddfast_str_basic& cmp, std::vector<ddfast_str_basic>& out) const;
    template<class F>
    void split_ex(F finder, std::vector<ddfast_str_basic>& out) const;

private:
    const t* m_buff;
    t_size m_l_pos = 0; // 包含
    t_size m_r_pos = 0; // 不包含
};

template <class t>
ddfast_str_basic<t>::ddfast_str_basic(const t* buff) : m_buff(buff), m_l_pos(0)
{
    DDASSERT(buff != nullptr);
    t_str tmp(m_buff);
    m_r_pos = tmp.length();
}

template <class t>
ddfast_str_basic<t>::ddfast_str_basic(const t_str& str) :
    m_buff(str.c_str()), m_l_pos(0), m_r_pos(str.length()) { }

template <class t>
ddfast_str_basic<t>::ddfast_str_basic(const t* buff, t_size len) :
    m_buff(buff), m_l_pos(0), m_r_pos(len) 
{
    DDASSERT(buff != nullptr);
}

template <class t>
ddfast_str_basic<t>::ddfast_str_basic(const t* buff, t_size l, t_size r) :
    m_buff(buff), m_l_pos(l), m_r_pos(r)
{
    DDASSERT(buff != nullptr);
    DDASSERT(l <= r);
}

template <class t>
inline const t* ddfast_str_basic<t>::data() const
{
    return m_buff;
}

template <class t>
inline typename ddfast_str_basic<t>::t_size ddfast_str_basic<t>::length() const
{
    return m_r_pos - m_l_pos;
}

template <class t>
inline t ddfast_str_basic<t>::operator[](t_size index) const
{
    DDASSERT(index < length());
    return m_buff[m_l_pos + index];
}

template <class t>
inline bool ddfast_str_basic<t>::operator==(const ddfast_str_basic& r) const
{
    if (length() != r.length()) {
        return false;
    }

    for (t_size i = 0; i < length(); ++i) {
        if ((*this)[i] != r[i]) {
            return false;
        }
    }

    return true;
}

template <class t>
inline bool ddfast_str_basic<t>::operator!=(const ddfast_str_basic& r) const
{
    return !(*this == r);
}

template <class t>
inline typename ddfast_str_basic<t>::t_str ddfast_str_basic<t>::to_str() const
{
    return t_str(m_buff + m_l_pos, m_buff + m_r_pos);
}

template <class t>
inline const t* ddfast_str_basic<t>::c_str() const
{
    return m_buff + m_l_pos;
}

template <class t>
void ddfast_str_basic<t>::trim()
{
    trim(' ');
}

template <class t>
void ddfast_str_basic<t>::trim(t c)
{
    for (t_size i = m_l_pos; i < m_r_pos; ++i) {
        if (m_buff[i] != c) {
            break;
        }

        ++m_l_pos;
    }

    for (t_size i = m_r_pos; i > m_l_pos + 1; --i) {
        if (m_buff[i - 1] != c) {
            break;
        }
        --m_r_pos;
    }
}

template <class t>
void ddfast_str_basic<t>::trim(const std::vector<t>& trims)
{
    std::vector<bool> trims_char(256, false);
    for (size_t i = 0; i < trims.size(); ++i) {
        trims_char[trims[i]] = true;
    }

    for (t_size i = m_l_pos; i < m_r_pos; ++i) {
        if (!trims_char[m_buff[i]]) {
            break;
        }

        ++m_l_pos;
    }

    for (t_size i = m_r_pos; i > m_l_pos + 1; --i) {
        if (!trims_char[m_buff[i - 1]]) {
            break;
        }
        --m_r_pos;
    }
}

template <class t>
void ddfast_str_basic<t>::trim(const ddfast_str_basic& trim)
{
    if (trim.length() == 0) {
        return;
    }
    while (beg_with(trim)) {
        to_sub_str(trim.length());
    }

    while (end_with(trim)) {
        to_sub_str(0, length() - trim.length());
    }
}

template <class t>
bool ddfast_str_basic<t>::beg_with(const ddfast_str_basic& cmp) const
{
    if (length() < cmp.length()) {
        return false;
    }

    for (t_size i = 0; i < cmp.length(); ++i) {
        if ((*this)[i] != cmp[i]) {
            return false;
        }
    }

    return true;
}

template <class t>
bool ddfast_str_basic<t>::end_with(const ddfast_str_basic& cmp) const
{
    if (length() < cmp.length()) {
        return false;
    }

    for (t_size i = 0; i < cmp.length(); ++i) {
        if ((*this)[length() - cmp.length() + i] != cmp[i]) {
            return false;
        }
    }

    return true;
}

template <class t>
ddfast_str_basic<t> ddfast_str_basic<t>::sub_str(t_size l, t_size len/* = xnpos*/) const
{
    DDASSERT(l <= length());
    if (len == xnpos) {
        return ddfast_str_basic(m_buff, m_l_pos + l, m_r_pos);
    } else {
        DDASSERT(l + len <= m_r_pos);
        return ddfast_str_basic(m_buff, m_l_pos + l, m_l_pos + l + len);
    }
}

template <class t>
void ddfast_str_basic<t>::to_sub_str(ddfast_str_basic<t>::t_size l, ddfast_str_basic<t>::t_size len/* = xnpos*/)
{
    DDASSERT(l <= length());
    m_l_pos += l;
    if (len != xnpos) {
        DDASSERT(l + len <= m_r_pos);
        m_r_pos = m_l_pos + len;
    }
}

template <class t>
typename ddfast_str_basic<t>::t_size ddfast_str_basic<t>::find(t c) const
{
    for (t_size i = 0; i < length(); ++i) {
        if ((*this)[i] == c) {
            return i;
        }
    }

    return xnpos;
}

template <class t>
typename ddfast_str_basic<t>::t_size ddfast_str_basic<t>::find(const ddfast_str_basic& cmp) const
{
    if (cmp.length() == 0) {
        return 0;
    }

    if (cmp.length() > length()) {
        return xnpos;
    }

    for (t_size i = 0; i < length(); ++i) {
        if ((*this)[i] != cmp[0]) {
            continue;
        }

        if (sub_str(i).beg_with(cmp)) {
            return i;
        }
    }

    return xnpos;
}

template <class t>
typename ddfast_str_basic<t>::t_size ddfast_str_basic<t>::find_last_of(t c) const
{
    for (t_size i = length(); i > 0; --i) {
        if ((*this)[i - 1] == c) {
            return i - 1;
        }
    }

    return xnpos;
}


template <class t>
typename ddfast_str_basic<t>::t_size ddfast_str_basic<t>::find_last_of(const ddfast_str_basic& cmp) const
{
    if (cmp.length() == 0) {
        return xnpos;
    }

    if (cmp.length() > length()) {
        return xnpos;
    }

    for (t_size i = length() - cmp.length() + 1; i > 0; --i) {
        if ((*this)[i - 1] != cmp[0]) {
            continue;
        }

        if (sub_str(i - 1).beg_with(cmp)) {
            return i - 1;
        }
    }

    return xnpos;
}

template <class t>
void ddfast_str_basic<t>::split(const ddfast_str_basic& cmp, std::vector<ddfast_str_basic>& out) const
{
    DDASSERT(cmp.length() > 0);
    ddfast_str_basic src(*this);
    t_size pos = src.find(cmp);
    while (pos != xnpos)
    {
        if (pos != 0) {
            out.emplace_back(src.sub_str(0, pos));
        }
        src.to_sub_str(pos + cmp.length());
        pos = src.find(cmp);
    }

    if (src.length() != 0) {
        out.emplace_back(src);
    }
}

template <class t>
template<class F>
void ddfast_str_basic<t>::split_ex(F finder, std::vector<ddfast_str_basic>& out) const
{
    ddfast_str_basic src(*this);
    std::tuple<t_size, t_size> find = finder(src);
    t_size pos = std::get<0>(find);
    while (pos != xnpos)
    {
        if (pos != 0) {
            out.emplace_back(src.sub_str(0, pos));
        }
        t_size length = std::get<1>(find);
        src.to_sub_str(pos + length);
        find = finder(src);
        pos = std::get<0>(find);
    }

    if (src.length() != 0) {
        out.emplace_back(src);
    }
}

using ddfast_str = ddfast_str_basic<char>;
using ddfast_strw = ddfast_str_basic<wchar_t>;

} // namespace NSP_DD
#endif // ddbase_str_ddfast_str_hpp_
