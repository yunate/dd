
#ifndef ddbase_dddef_h_
#define ddbase_dddef_h_

#define NSP_DD dd

/////////////////////////////////////数字定义/////////////////////////////////////
// 无符号
#include <cstdint>
namespace NSP_DD {
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// 有符号
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
} // namespace NSP_DD

/////////////////////////////////////string/////////////////////////////////////
#include <string>
template<class _Elem>
using ddstrt = std::basic_string<_Elem, std::char_traits<_Elem>, std::allocator<_Elem>>;

/////////////////////////////////////buff/////////////////////////////////////
#include <vector>
namespace NSP_DD {
using ddbuff = std::vector<u8>;

template<class t>
using ddbufft = std::vector<t>;
} // namespace NSP_DD

/////////////////////////////////////development/////////////////////////////////////
#endif // ddbase_dddef_h_


