
#ifndef ddbase_str_ddurl_parser_h_
#define ddbase_str_ddurl_parser_h_

#include "ddbase/dddef.h"

#include <regex>
namespace NSP_DD {
/** url
一般以utf-8编码
scheme://[user[:password]@]host[:port] [/path] [?query] [#fragment]
*/
struct ddurl
{
    /** 指定低层使用的协议(例如：http, https, ftp)
    */
    std::string m_scheme;

    /** 用户名称
    @note: [可省略]
    */
    std::string m_user;

    /** 密码
    @note: [可省略]
    */
    std::string m_password;

    /** 服务器。通常为域名，有时为IP地址
    */
    std::string m_host;

    /** 端口。以数字方式表示,默认值“:80”
    @note: [可省略]
    */
    unsigned int m_port = 0;

    /** 路径。以“/”字符区别路径中的每一个目录名称
    @note: [可省略]
    */
    std::string m_path;

    /** 查询。GET模式的窗体参数，以“?”字符为起点，每个参数以“&”隔开，再以“=”分开参数名称与数据，通常以UTF8的URL编码，避开字符冲突的问题
    @note: [可省略]
    */
    std::string m_query;

    /** 片段。以“#”字符为起点
    @note: [可省略]
    */
    std::string m_fragment;

    /** url 是否有效
    @return 是否有效
    */
    bool is_valid() {
        return !m_scheme.empty() && m_port != 0 && !m_host.empty();
    }

    /** 获得格式化后的URL
    @note scheme:[//[user[:password]@]host[:port]] [/path] [?query] [#fragment]
    @return 格式化后的URL
    */
    std::string get_formated_url()
    {
        std::string formatedUrl = "";

        if (!is_valid()) {
            return formatedUrl;
        }

        formatedUrl += m_scheme;
        formatedUrl += "://";

        if (m_user.length() != 0 || m_password.length() != 0) {
            formatedUrl += m_user;

            if (m_password.length() != 0) {
                formatedUrl += ":";
                formatedUrl += m_password;
            }
            
            formatedUrl += "@";
        }

        formatedUrl += m_host;

        if (!( (m_port == 80) &&
               (m_scheme.length() >= 4) &&
               (m_scheme[0] == 'h' || m_scheme[0] == 'H') &&
               (m_scheme[1] == 't' || m_scheme[1] == 'T') &&
               (m_scheme[2] == 't' || m_scheme[2] == 'T') &&
               (m_scheme[3] == 'p' || m_scheme[3] == 'P') )) {
            formatedUrl += ":";
            char portBuff[10] = {0};
            ::_itoa_s(m_port, portBuff, 10);
            formatedUrl += portBuff;
        }

        if (m_path.length() > 0) {
            formatedUrl += m_path;
        }

        if (m_query.length() > 0) {
            formatedUrl += "?";
            formatedUrl += m_query;
        }

        if (m_fragment.length() > 0) {
            formatedUrl += "#";
            formatedUrl += m_fragment;
        }

        return formatedUrl;
    }
};

/** 解析url
@note: 是否成功用dogUrl 的 is_valid 判断
@param [in] url url原始字符串（一般以UTF-8编码）
@param [out] dogUrl 输出
*/
inline void parse_url(const std::string& url, ddurl& dogUrl)
{
    size_t index = 0;
    size_t len = url.length();

    // 找scheme
    for (size_t i = index; i < len; ++i) {
        if (url[i] == ':') {
            dogUrl.m_scheme = url.substr(index, i - index);
            index = i + 1;
            break;
        }
    }

    if (dogUrl.m_scheme.empty()) {
        return;
    }

    // 固定 // 或者 \\ 
    if (index + 1 >= len ||
        (url[index] != '/' && url[index] != '\\') ||
        (url[index + 1] != '/' && url[index + 1] != '\\')) {
        return;
    }

    // 指向 “/”
    index += 1;

    // [user[:password]@]host[:port] 【[/path] [?query] [#fragment]】
    // 找到扩展符号 “/ \ ? #”
    size_t hostPortEndIndex = len - 1;
    for (size_t i = index + 1; i < len; ++i) {
        if (url[i] == '/' || url[i] == '\\' || url[i] == '?' || url[i] == '#') {
            hostPortEndIndex = i - 1;
            break;
        }
    }

    // 长度为0
    if (hostPortEndIndex <= index) {
        return;
    }

    // 找到@符号，说明有用户名密码
    {
        size_t userPswEndIndex = 0;
        for (size_t i = index + 1; i <= hostPortEndIndex; ++i) {
            if (url[i] == '@') {
                userPswEndIndex = i - 1;

                // 长度不为0
                if (userPswEndIndex > index) {
                    // 寻找 “:”
                    size_t userEndIndex = userPswEndIndex;
                    for (size_t j = index + 1; j <= userPswEndIndex; ++j) {
                        if (url[j] == ':') {
                            userEndIndex = j - 1;
                            break;
                        }
                    }

                    dogUrl.m_user = url.substr(index + 1, userEndIndex - index);

                    if (userEndIndex < userPswEndIndex - 2) {
                        dogUrl.m_password = url.substr(userEndIndex + 2, userPswEndIndex - userEndIndex - 1);
                    }
                }

                // 指向 “@”
                index = userPswEndIndex + 1;
                break;
            }
        }
    }

    // host 端口
    {
        if (index < hostPortEndIndex) {
            // 寻找 “:”
            size_t hostEndIndex = hostPortEndIndex;
            for (size_t j = index + 1; j <= hostPortEndIndex; ++j) {
                if (url[j] == ':') {
                    hostEndIndex = j - 1;
                    break;
                }
            }

            // 长度为0
            if (hostEndIndex < index) {
                return;
            }

            dogUrl.m_host = url.substr(index + 1, hostEndIndex - index);

            if (hostEndIndex < hostPortEndIndex - 2) {
                dogUrl.m_port = ::atoi(url.substr(hostEndIndex + 2, hostPortEndIndex - hostEndIndex - 1).c_str());
            }

            if (dogUrl.m_port == 0) {
                dogUrl.m_port = 80;
            }

            index = hostPortEndIndex + 1;
        } else {
            return;
        }
    }

    while (index < len) {
        // 寻找下一个扩展符号
        size_t extendEndIndex = len - 1;
        for (size_t i = index + 1; i < len; ++i) {
            if (url[i] == '?' || url[i] == '#' || url[i] == '/' || url[i] == '\\') {
                extendEndIndex = i - 1;
                break;
            }
        }

        // 如果上一个符号是 "/" 或者 "\\" 那么填充填充m_path
        if (url[index] == '/' || url[index] == '\\') {
            // 继续寻找直到 ? #
            size_t i = extendEndIndex;
            extendEndIndex = len - 1;
            for (; i < len; ++i) {
                if (url[i] == '?' || url[i] == '#') {
                    extendEndIndex = i - 1;
                    break;
                }
            }

            dogUrl.m_path = url.substr(index, extendEndIndex - index + 1);
            size_t pathEndIndex = 0;

            for (size_t ii = 0; ii < dogUrl.m_path.length(); ++ii) {
                if (dogUrl.m_path[ii] == '\\' || dogUrl.m_path[ii] == '/') {
                    if (pathEndIndex == 0 || dogUrl.m_path[pathEndIndex - 1] != '/') {
                        dogUrl.m_path[pathEndIndex++] = '/';
                    }
                } else {
                    dogUrl.m_path[pathEndIndex++] = dogUrl.m_path[ii];
                }
            }

            dogUrl.m_path[pathEndIndex] = 0;
            dogUrl.m_path = dogUrl.m_path.c_str();
        } else if (url[index] == '?') {
            // 如果上一个符号是 "?" 那么填充m_query
            dogUrl.m_query = url.substr(index + 1, extendEndIndex - index);
        } else if (url[index] == '#') {
            // 如果上一个符号是 "#" 那么填充m_fragment
            dogUrl.m_fragment = url.substr(index + 1, extendEndIndex - index);
        }

        index = extendEndIndex + 1;
    }
}

/** 正则表达式解析url,和parse_url稍有不同parse_urlRegex对[/path] [?query] [#fragment]的顺序严格要求
scheme://[user[:password]@]host[:port] [/path] [?query] [#fragment]
@note: 是否成功用dogUrl 的 is_valid 判断
@param [in] url url原始字符串（一般以UTF-8编码）
@param [out] dogUrl 输出
*/
inline void parse_url_regex(const std::string& url, ddurl& dogUrl)
{
    // note: 正则表达式中 方括号中"\"和"]"需要转义，其他的不需要
    std::string parttenStr;

    // scheme://
    parttenStr += R"__(([a-zA-Z]+):[\\/]{2})__";

    // [user[:password]@]
    parttenStr += R"__((?:([^:@]*)(?::([^@]*))?@)?)__";

    // host[:port]
    parttenStr += R"__(([^/\\#?:]*)(?::([0-9]{0,5}))?)__";

    // [/path]
    parttenStr += R"__((?:([/\\][^?#]*))?)__";

    // [?query]
    parttenStr += R"__((?:[?]([^?#/\\]*))?)__";

    // [#fragment]
    parttenStr += R"__((?:[#]([^?#/\\]*))?)__";

    std::regex partten(parttenStr);
    std::smatch results;

    if (!std::regex_search(url, results, partten) || results.size() != 9) {
        return;
    }

    dogUrl.m_scheme = results[1];
    dogUrl.m_user = results[2];
    dogUrl.m_password = results[3];
    dogUrl.m_host = results[4];
    dogUrl.m_port = ::atoi(results[5].str().c_str());
    dogUrl.m_path = results[6];
    dogUrl.m_query = results[7];
    dogUrl.m_fragment = results[8];

    size_t pathEndIndex = 0;

    for (size_t i = 0; i < dogUrl.m_path.length(); ++i) {
        if (dogUrl.m_path[i] == '\\' || dogUrl.m_path[i] == '/') {
            if (pathEndIndex == 0 || dogUrl.m_path[pathEndIndex - 1] != '/') {
                dogUrl.m_path[pathEndIndex++] = '/';
            }
        } else {
            dogUrl.m_path[pathEndIndex++] = dogUrl.m_path[i];
        }
    }

    dogUrl.m_path[pathEndIndex] = 0;
    dogUrl.m_path = dogUrl.m_path.c_str();

    if (dogUrl.m_port == 0) {
        dogUrl.m_port = 80;
    }
}

} // namespace NSP_DD
#endif // ddbase_str_ddurl_parser_h_
