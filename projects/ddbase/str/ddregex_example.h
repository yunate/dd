
#ifndef ddbase_str_ddregex_example_h_
#define ddbase_str_ddregex_example_h_

//////////////////////////////////////////////////////////////////////////
//                         一些常用的正则表达式                             //
//                         仅供参考！！！                                  //
//////////////////////////////////////////////////////////////////////////

#include "ddbase/dddef.h"
namespace NSP_DD {

/** 数字：
*/
static const wchar_t* s_regex_number = _T(R"([0-9]*)");

/** n 位的数字
*/
static const wchar_t* s_regex_n_number = _T(R"(\d{n})");

/** 至少 n 位的数字
*/
static const wchar_t* s_regex_np_number = _T(R"(\d{n,})");

/** m-n 位的数字：
*/
static const wchar_t* s_regex_nm_number = _T(R"(\d{n,m})");

/** 零开头的数字
*/
static const wchar_t* s_regex_0_number = _T(R"((0[0-9]*))");

/** 非零开头的数字
*/
static const wchar_t* s_regex_n0_number = _T(R"(([1-9][0-9]*))");

/** 非零开头的最多带m位小数的数字
*/
static const wchar_t* s_regex_n0_mdcml_number = _T(R"(([1-9][0-9]*)+(\.[0-9]{1,m})?)");

/** 最多带m位小数的负数
*/
static const wchar_t* s_regex_nativ_mdcml_number = _T(R"((\-)?\d+(\.\d{1,m})?)");

/** 有理数
*/
static const wchar_t* s_regex_q_number = _T(R"((\-|\+)?\d+(\.\d+)?)");

/** 汉字
*/
static const wchar_t* s_regex_chinese = _T(R"([\u4e00-\u9fa5]+)");

/** 英文和数字
*/
static const wchar_t* s_regex_comm_char = _T(R"([A-Za-z0-9]+)");

/** Email 地址：?
*/
static const wchar_t* s_regex_email = _T(R"([a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\.[a-zA-Z0-9_-]+)+)");

/** 域名：?
*/
static const wchar_t* s_regex_domain_name = _T(R"(\w+(\.\w)+)");

} // namespace NSP_DD
#endif // ddbase_str_ddregex_example_h_
