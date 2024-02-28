#include "ddbase/stdafx.h"
#include "ddbase/ddlocale.h"

#include "ddbase/str/ddstr.h"
#include "ddbase/dddef.h"

#include <codecvt>
#include <windows.h>

namespace NSP_DD {
void ddstr_::format(const char* format, std::string& out, va_list args)
{
    out.clear();
    const size_t buff_size = 1024;
    char buff[buff_size] = { 0 };
    size_t len = ::vsnprintf(buff, buff_size, format, args);
    if (len == -1) {
        return;
    }
    if (len < buff_size) {
        out = buff;
        return;
    }
    out.resize(len);
    if (::vsnprintf((char*)out.data(), out.size() + 1, format, args) == -1) {
        out.clear();
        return;
    }

    return;
}
void ddstr_::format(const wchar_t* format, std::wstring& out, va_list args)
{
#pragma warning(push)
#pragma warning(disable:4996)
    out.clear();
    const size_t buff_size = 1024;
    wchar_t buff[buff_size] = { 0 };
    size_t len = ::_vsnwprintf(buff, buff_size, format, args);
    if (len != -1 && len < buff_size) {
        out = buff;
        return;
    }

    if (len == -1) {
        len = ::_vsnwprintf(nullptr, 0, format, args);
        if (len == -1) {
            return;
        }
    }

    out.resize(len);
    if (::_vsnwprintf((wchar_t*)out.data(), out.size() + 1, format, args) == -1) {
        out.clear();
        return;
    }
    return;
#pragma warning(pop)
}


static bool utf16_8_core(char* dest, size_t* destLen, const wchar_t* src, size_t srcLen)
{
    *destLen = 0;
    for (size_t i = 0; i < srcLen; ++i) {
        unsigned long unic = src[i];
        if (unic <= 0x0000007F) {
            // * U-00000000 - U-0000007F:  0xxxxxxx
            if (!dest) {
                (*destLen) += 1;
                continue;
            }

            dest[(*destLen)++] = (unic & 0x7F);
        } else if (unic >= 0x00000080 && unic <= 0x000007FF) {
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            if (!dest) {
                (*destLen) += 2;
                continue;
            }
            dest[(*destLen)++] = ((unic >> 6) & 0x1F) | 0xC0;
            dest[(*destLen)++] = (unic & 0x3F) | 0x80;
        } else if (unic >= 0x00000800 && unic <= 0x0000FFFF) {
            // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            if (!dest)
            {
                (*destLen) += 3;
                continue;
            }

            dest[(*destLen)++] = ((unic >> 12) & 0x0F) | 0xE0;
            dest[(*destLen)++] = ((unic >> 6) & 0x3F) | 0x80;
            dest[(*destLen)++] = (unic & 0x3F) | 0x80;
        } else if (unic >= 0x00010000 && unic <= 0x001FFFFF) {
            // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            if (!dest) {
                (*destLen) += 4;
                continue;
            }

            dest[(*destLen)++] = ((unic >> 18) & 0x07) | 0xF0;
            dest[(*destLen)++] = ((unic >> 12) & 0x3F) | 0x80;
            dest[(*destLen)++] = ((unic >> 6) & 0x3F) | 0x80;
            dest[(*destLen)++] = (unic & 0x3F) | 0x80;
        } else if (unic >= 0x00200000 && unic <= 0x03FFFFFF) {
            // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            if (!dest) {
                (*destLen) += 5;
                continue;
            }

            dest[(*destLen)++] = ((unic >> 24) & 0x03) | 0xF8;
            dest[(*destLen)++] = ((unic >> 18) & 0x3F) | 0x80;
            dest[(*destLen)++] = ((unic >> 12) & 0x3F) | 0x80;
            dest[(*destLen)++] = ((unic >> 6) & 0x3F) | 0x80;
            dest[(*destLen)++] = (unic & 0x3F) | 0x80;
        } else if (unic >= 0x04000000 && unic <= 0x7FFFFFFF) {
            // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            if (!dest) {
                (*destLen) += 6;
                continue;
            }

            dest[(*destLen)++] = ((unic >> 30) & 0x01) | 0xFC;
            dest[(*destLen)++] = ((unic >> 24) & 0x3F) | 0x80;
            dest[(*destLen)++] = ((unic >> 18) & 0x3F) | 0x80;
            dest[(*destLen)++] = ((unic >> 12) & 0x3F) | 0x80;
            dest[(*destLen)++] = ((unic >> 6) & 0x3F) | 0x80;
            dest[(*destLen)++] = (unic & 0x3F) | 0x80;
        }
    }

    return (*destLen != 0);
}

const u8 kUtf8Limits[5] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static bool utf8_16_core(wchar_t* dest, size_t* destLen, const char* src, size_t srcLen)
{
    size_t destPos = 0, srcPos = 0;
    for (;;) {
        u8 c;
        unsigned numAdds;
        if (srcPos == srcLen) {
            *destLen = destPos;
            return true;
        }
        c = (u8)src[srcPos++];
        if (c < 0x80) {
            if (dest)
                dest[destPos] = (wchar_t)c;
            destPos++;
            continue;
        }
        if (c < 0xC0) {
            break;
        }
        for (numAdds = 1; numAdds < 5; numAdds++) {
            if (c < kUtf8Limits[numAdds]) {
                break;
            }
        }
        u32 value = (c - kUtf8Limits[numAdds - 1]);
        do {
            u8 c2;
            if (srcPos == srcLen) {
                break;
            }
            c2 = (u8)src[srcPos++];
            if (c2 < 0x80 || c2 >= 0xC0) {
                break;
            }
            value <<= 6;
            value |= (c2 - 0x80);
        } while (--numAdds);

        if (value < 0x10000) {
            if (dest) {
                dest[destPos] = (wchar_t)value;
            }
            destPos++;
        } else {
            value -= 0x10000;
            if (value >= 0x100000) {
                break;
            }
            if (dest) {
                dest[destPos + 0] = (wchar_t)(0xD800 + (value >> 10));
                dest[destPos + 1] = (wchar_t)(0xDC00 + (value & 0x3FF));
            }
            destPos += 2;
        }
    }
    *destLen = destPos;
    return false;
}

bool ddstr::utf16_8(const std::wstring& src, std::string& des)
{
    size_t destLen = 0;
    if (!utf16_8_core(nullptr, &destLen, src.c_str(), src.length())) {
        return false;
    }

    des.resize(destLen);
    return utf16_8_core((char*)des.c_str(), &destLen, src.c_str(), src.length());
}

std::string ddstr::utf16_8(const std::wstring& src)
{
    std::string dst;
    (void)utf16_8(src, dst);
    return dst;
}

bool ddstr::utf8_16(const std::string& src, std::wstring& des)
{
 
    size_t destLen = 0;
    if (!utf8_16_core(nullptr, &destLen, src.c_str(), src.length())) {
        return false;
    }

    des.resize(destLen);
    return utf8_16_core((wchar_t*)des.c_str(), &destLen, src.c_str(), src.length());
}

std::wstring ddstr::utf8_16(const std::string& src)
{
    std::wstring dst;
    (void)utf8_16(src, dst);
    return dst;
}

bool ddstr::to_utf16(const std::string& src, std::wstring& des, u32 code_page)
{
    int len = ::MultiByteToWideChar((UINT)code_page, 0, src.c_str(), -1, NULL, 0);
    if (len <= 0) {
        return false;
    }

    // len 包括了末尾的'\0', 因此减掉一
    des.resize((size_t)len - 1);
    return ::MultiByteToWideChar((UINT)code_page, 0, src.c_str(), -1, (wchar_t*)des.data(), len) > 0;
}

std::wstring ddstr::to_utf16(const std::string& src, u32 code_page)
{
    std::wstring dst;
    (void)to_utf16(src, dst, code_page);
    return dst;
}

bool ddstr::utf16_to(const std::wstring& src, std::string& des, u32 code_page)
{
    int len = ::WideCharToMultiByte((UINT)code_page, 0, src.data(), -1, NULL, 0, NULL, NULL);
    if (len <= 0) {
        return false;
    }

    des.resize((size_t)len - 1);
    return ::WideCharToMultiByte((UINT)code_page, 0, src.data(), -1, (char*)des.data(), len, NULL, NULL) > 0;
}

std::string ddstr::utf16_to(const std::wstring& src, u32 code_page)
{
    std::string dst;
    (void)utf16_to(src, dst, code_page);
    return dst;
}

bool ddstr::to_utf8(const std::string& src, std::string& des, u32 code_page)
{
    std::wstring wdes;
    if (!to_utf16(src, wdes, code_page)) {
        return false;
    }

    return utf16_8(wdes, des);
}

std::string ddstr::to_utf8(const std::string& src, u32 code_page)
{
    std::string des;
    (void)to_utf8(src, des, code_page);
    return des;
}

bool ddstr::utf8_to(const std::string& src, std::string& des, u32 code_page)
{
    std::wstring wdes;
    if (!utf8_16(src, wdes)) {
        return false;
    }

    return utf16_to(wdes, des, code_page);
}

std::string ddstr::utf8_to(const std::string& src, u32 code_page)
{
    std::string des;
    (void)utf8_to(src, des, code_page);
    return des;
}

bool ddstr::utf16_ansi(const std::wstring& src, std::string& des, bool system_ansi /* = true */)
{
    if (!system_ansi) {
        return utf16_to(src, des, ddlocale::get_current_locale_codepage_id());
    } else {
        return utf16_to(src, des, CP_ACP);
    }
}

std::string ddstr::utf16_ansi(const std::wstring& src, bool system_ansi /* = true */)
{
    std::string dst;
    (void)utf16_ansi(src, dst, system_ansi);
    return dst;
}

bool ddstr::ansi_utf16(const std::string& src, std::wstring& des, bool system_ansi /* = true */)
{
    if (!system_ansi) {
        return to_utf16(src, des, ddlocale::get_current_locale_codepage_id());
    } else {
        return to_utf16(src, des, CP_ACP);
    }
}

std::wstring ddstr::ansi_utf16(const std::string& src, bool system_ansi /* = true */)
{
    std::wstring dst;
    (void)ansi_utf16(src, dst, system_ansi);
    return dst;
}

bool ddstr::utf8_ansi(const std::string& src, std::string& des)
{
    return utf8_to(src, des, CP_ACP);
}

std::string ddstr::utf8_ansi(const std::string& src)
{
    std::string dst;
    (void)utf8_ansi(src, dst);
    return dst;
}

bool ddstr::ansi_utf8(const std::string& src, std::string& des)
{
    return to_utf8(src, des, CP_ACP);
}

std::string ddstr::ansi_utf8(const std::string& src)
{
    std::string dst;
    (void)ansi_utf8(src, dst);
    return dst;
}

template<class _Elem>
static void to_lower_t(_Elem* src)
{
    DDASSERT(src != nullptr);
    while (true) {
        if (*src == 0) {
            break;
        }

        *src = (_Elem)std::tolower(*src);
        ++src;
    }
}

void ddstr::to_lower(char* src)
{
    to_lower_t(src);
}

void ddstr::to_lower(wchar_t* src)
{
    to_lower_t(src);
}

void ddstr::to_lower(std::string& src)
{
    to_lower(const_cast<char*>(src.c_str()));
}

void ddstr::to_lower(std::wstring& src)
{
    to_lower(const_cast<wchar_t*>(src.c_str()));
}

std::string ddstr::lower(const char* src)
{
    std::string tmp(src);
    to_lower(tmp);
    return tmp;
}

std::wstring ddstr::lower(const wchar_t* src)
{
    std::wstring tmp(src);
    to_lower(tmp);
    return tmp;
}

template<class _Elem>
static void to_upper_t(_Elem* src)
{
    DDASSERT(src != nullptr);
    while (true) {
        if (*src == 0) {
            break;
        }

        *src = (_Elem)std::toupper(*src);
        ++src;
    }
}

void ddstr::to_upper(char* src)
{
    to_upper_t(src);
}

void ddstr::to_upper(wchar_t* src)
{
    to_upper_t(src);
}

void ddstr::to_upper(std::string& src)
{
    to_upper(const_cast<char*>(src.c_str()));
}

void ddstr::to_upper(std::wstring& src)
{
    to_upper(const_cast<wchar_t*>(src.c_str()));
}

std::string ddstr::upper(const char* src)
{
    std::string tmp(src);
    to_upper(tmp);
    return tmp;
}

std::wstring ddstr::upper(const wchar_t* src)
{
    std::wstring tmp(src);
    to_upper(tmp);
    return tmp;
}

template<class _Elem>
static s32 strcmp_t(const _Elem* src, const _Elem* find)
{
    DDASSERT(src != nullptr);
    DDASSERT(find != nullptr);
    while (*src == *find) {
        if (0 == *src) {
            return 0;
        }

        ++src;
        ++find;
    }

    return (s32)(*src - *find);
}
s32 ddstr::strcmp(const char* src, const char* find)
{
    return strcmp_t(src, find);
}
s32 ddstr::strcmp(const wchar_t* src, const wchar_t* find)
{
    return strcmp_t(src, find);
}

template<class _Elem>
static s32 strncmp_t(const _Elem* src, const _Elem* find, u32 n)
{
    DDASSERT(src != nullptr);
    DDASSERT(find != nullptr);
    if (n == 0) {
        return 0;
    }

    while (*src == *find) {
        --n;
        if (0 == n || 0 == *src) {
            return 0;
        }

        ++src;
        ++find;
    }

    return (s32)(*src - *find);
}
s32 ddstr::strncmp(const char* src, const char* find, u32 n)
{
    return strncmp_t(src, find, n);
}
s32 ddstr::strncmp(const wchar_t* src, const wchar_t* find, u32 n)
{
    return strncmp_t(src, find, n);
}

template<class _Elem>
static u32 strlen_t(const _Elem* src)
{
    DDASSERT(src != nullptr);
    u32 len = 0;
    while (*src ++ != 0) {
        ++len;
    }

    return len;
}

template<class _Elem>
static bool endwith_t(const _Elem* src, const _Elem* find)
{
    DDASSERT(src != nullptr);
    DDASSERT(find != nullptr);
    u32 l = strlen_t(src);
    u32 r = strlen_t(find);
    if (l < r) {
        return false;
    }

    return (strcmp_t((src + l - r), find) == 0);
}
bool ddstr::endwith(const char* src, const char* find)
{
    return endwith_t(src, find);
}
bool ddstr::endwith(const wchar_t* src, const wchar_t* find)
{
    return endwith_t(src, find);
}

template<class _Elem>
static bool beginwith_t(const _Elem* src, const _Elem* find)
{
    DDASSERT(src != nullptr);
    DDASSERT(find != nullptr);
    while (*find == *src) {
        if (0 == *find) {
            return true;
        }
        ++find;
        ++src;
    }
    return *find == 0;
}
bool ddstr::beginwith(const char* src, const char* find)
{
    return beginwith_t(src, find);
}
bool ddstr::beginwith(const wchar_t* src, const wchar_t* find)
{
    return beginwith_t(src, find);
}

template<class _Elem>
static const _Elem* strstr_t(const _Elem* src, const _Elem* find)
{
    DDASSERT(src != nullptr);
    DDASSERT(find != nullptr);
    while (!beginwith_t(src, find)) {
        if (*src == 0) {
            return nullptr;
        }
        ++src;
    }

    return src;
}
const char* ddstr::strstr(const char* src, const char* find)
{
    return strstr_t(src, find);
}
const wchar_t* ddstr::strstr(const wchar_t* src, const wchar_t* find)
{
    return strstr_t(src, find);
}

template<class _Elem>
static bool strwildcard_t(const _Elem* str, const _Elem* find)
{
    DDASSERT(str != nullptr);
    DDASSERT(find != nullptr);
    const _Elem* back_find = nullptr;
    const _Elem* back_str = nullptr;

    for (;;)
    {
        _Elem c = *str++;
        _Elem d = *find++;

        switch (d)
        {
        case '?':
            {
                if (c == '\0') {
                    return false;
                }

                break;
            }
        case '*':
            {
                if (*find == '\0') {
                    return true;
                }

                back_find = find;
                back_str = --str;
                break;
            }
        default:
            {
                if (c == d) {
                    if (d == '\0') {
                        return true;
                    }

                    break;
                }

                if (c == '\0' || !back_find) {
                    return false;
                }

                find = back_find;
                str = ++back_str;
                break;
            }
        }
    }
}
bool ddstr::strwildcard(const char* src, const char* find)
{
    return strwildcard_t(src, find);
}
bool ddstr::strwildcard(const wchar_t* src, const wchar_t* find)
{
    return strwildcard_t(src, find);
}


template<class _Elem>
void trim_t(std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>& src, _Elem c)
{
    size_t l_pos = 0;
    size_t r_pos = src.length();
    for (size_t i = l_pos; i < r_pos; ++i) {
        if (src[i] != c) {
            break;
        }

        ++l_pos;
    }

    for (size_t i = r_pos; i > l_pos + 1; --i) {
        if (src[i - 1] != c) {
            break;
        }
        --r_pos;
    }
    src = src.substr(l_pos, r_pos - l_pos);
}

template<class _Elem>
void trim_t(std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>& src, const std::vector<_Elem>& trims)
{
    std::vector<bool> trims_char(256, false);
    for (size_t i = 0; i < trims.size(); ++i) {
        trims_char[(u8)trims[i]] = true;
    }

    size_t l_pos = 0;
    size_t r_pos = src.length();
    for (size_t i = l_pos; i < r_pos; ++i) {
        if (!trims_char[(u8)src[i]]) {
            break;
        }

        ++l_pos;
    }

    for (size_t i = r_pos; i > l_pos + 1; --i) {
        if (!trims_char[(u8)src[i - 1]]) {
            break;
        }
        --r_pos;
    }
    src = src.substr(l_pos, r_pos - l_pos);
}

template<class _Elem>
void trim_t(std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>& src, const std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>& trim)
{
    if (trim.length() == 0) {
        return;
    }
    while (beginwith_t(src.c_str(), trim.c_str())) {
        src = src.substr(trim.length());
    }

    while (endwith_t(src.c_str(), trim.c_str())) {
        src = src.substr(0, src.length() - trim.length());
    }
}

void ddstr::trim(std::string& src)
{
    trim_t(src, ' ');
}
void ddstr::trim(std::string& src, char trim)
{
    trim_t(src, trim);
}
void ddstr::trim(std::string& src, const std::vector<char>& trims)
{
    trim_t(src, trims);
}
void ddstr::trim(std::string& src, const std::string& trim)
{
    trim_t(src, trim);
}
void ddstr::trim(std::wstring& src)
{
    trim_t(src, L' ');
}
void ddstr::trim(std::wstring& src, wchar_t trim)
{
    trim_t(src, trim);
}
void ddstr::trim(std::wstring& src, const std::vector<wchar_t>& trims)
{
    trim_t(src, trims);
}
void ddstr::trim(std::wstring& src, const std::wstring& trim)
{
    trim_t(src, trim);
}

template<class _Elem>
static void split_t(const _Elem* str, const _Elem* find, std::vector<std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>>& out)
{
    DDASSERT(str != nullptr);
    DDASSERT(find != nullptr);
    DDASSERT(*find != 0);

    u32 find_len = strlen_t(find);
    using string_t = std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>;
    const _Elem* beg = str;
    while (*str) {
        if (beginwith_t(str, find)) {
            if (beg != str) {
                out.emplace_back(string_t(beg, str - beg));
            }
            str += find_len;
            beg = str;
        } else {
            ++str;
        }
    }

    if (*beg != 0) {
        out.emplace_back(string_t(beg));
    }
}
void ddstr::split(const char* str, const char* find, std::vector<std::string>& out)
{
    split_t(str, find, out);
}
void ddstr::split(const wchar_t* str, const wchar_t* find, std::vector<std::wstring>& out)
{
    split_t(str, find, out);
}

template<class _Elem>
static std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>> replace_t(const _Elem* src, const _Elem* find, const _Elem* replacer)
{
    DDASSERT(src != nullptr);
    DDASSERT(find != nullptr);
    DDASSERT(replacer != nullptr);
    DDASSERT(*find != 0);
    using string_t = std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>;
    string_t rtn;
    u32 find_len = strlen_t(find);
    const _Elem* beg = src;
    while (*src) {
        if (beginwith_t(src, find)) {
            rtn += (string_t(beg, src - beg) + replacer);
            src += find_len;
            beg = src;
        } else {
            ++src;
        }
    }
    rtn += string_t(beg);
    return rtn;
}
std::string ddstr::replace(const char* src, const char* find, const char* replacer)
{
    return replace_t(src, find, replacer);
}
std::wstring ddstr::replace(const wchar_t* src, const wchar_t* find, const wchar_t* replacer)
{
    return replace_t(src, find, replacer);
}

s32 ddstr::buff_find(const ddbuff& src, const ddbuff& finder)
{
    if (src.empty() || finder.empty() || src.size() < finder.size()) {
        return -1;
    }

    size_t pos = 0;
    u8 finder_first = finder[0];
    for (; pos < src.size() - finder.size() + 1; ++pos) {
        if (src[pos] == finder_first) {
            bool has_find = true;
            for (size_t i = 1; i < finder.size(); ++i) {
                if (finder[i] != src[pos + i]) {
                    has_find = false;
                    break;
                }
            }

            if (has_find) {
                return (s32)pos;
            }
        }
    }

    return -1;
}

bool ddstr::buff_replace(const ddbuff& src, const ddbuff& finder, const ddbuff& replacer, ddbuff& out)
{
    if (src.empty()) {
        return false;
    }

    out.resize(0);
    if (out.capacity() == 0) {
        out.reserve(src.size());
    }

    if (finder.empty() || src.size() < finder.size() || replacer.empty()) {
        std::copy(src.begin(), src.end(), std::back_inserter(out));
        return false;
    }

    bool find = false;
    size_t pos = 0;
    size_t pre_pos = 0;
    u8 finder_first = finder[0];
    for (; pos < src.size() - finder.size() + 1; ++pos) {
        if (src[pos] == finder_first) {
            bool has_find = true;
            for (size_t i = 1; i < finder.size(); ++i) {
                if (finder[i] != src[pos + i]) {
                    has_find = false;
                    break;
                }
            }

            if (has_find) {
                find = true;
                std::copy(src.begin() + pre_pos, src.begin() + pos, std::back_inserter(out));
                std::copy(replacer.begin(), replacer.end(), std::back_inserter(out));
                pre_pos = pos + finder.size();
                pos = pre_pos - 1;
            }
        }
    }

    if (pre_pos < src.size()) {
        std::copy(src.begin() + pre_pos, src.end(), std::back_inserter(out));
    }
    return find;
}

s32 ddstr::buff_replace_ex(const ddbuff& src, const std::vector<ddbuff>& finder, const std::vector<ddbuff>& replacer, ddbuff& out)
{
    if (src.empty()) {
        return 0;
    }

    out.resize(0);
    if (out.capacity() == 0) {
        out.reserve(src.size());
    }

    if (finder.empty() || finder.size() != replacer.size()) {
        std::copy(src.begin(), src.end(), std::back_inserter(out));
        return 0;
    }

    bool find = false;
    size_t pos = 0;
    size_t pre_pos = 0;
    for (; pos < src.size(); ++pos) {
        bool has_finder = false;
        u8 c = src[pos];
        size_t i = 0;
        for (; i < finder.size(); ++i) {
            if (c == finder[i][0] && finder[i].size() <= src.size() - pos) {
                bool sub_has_finder = true;
                for (size_t j = 1; j < finder[i].size(); ++j) {
                    if (finder[i][j] != src[pos + j]) {
                        sub_has_finder = false;
                        break;
                    }
                }

                if (sub_has_finder) {
                    has_finder = true;
                    break;
                }
            }
        }

        if (has_finder) {
            find = true;
            std::copy(src.begin() + pre_pos, src.begin() + pos, std::back_inserter(out));
            std::copy(replacer[i].begin(), replacer[i].end(), std::back_inserter(out));
            pre_pos = pos + finder[i].size();
            pos = pre_pos - 1;
        }
    }

    if (pre_pos < src.size()) {
        std::copy(src.begin() + pre_pos, src.end(), std::back_inserter(out));
    }
    return (s32)pre_pos;
}
} // namespace NSP_DD
