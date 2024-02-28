
#ifndef ddbase_str_ddurl_hpp_
#define ddbase_str_ddurl_hpp_

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
    std::string scheme;

    /** 用户名称
    @note: [可省略]
    */
    std::string user;

    /** 密码
    @note: [可省略]
    */
    std::string password;

    /** 服务器。通常为域名，有时为IP地址
    */
    std::string host;

    /** 端口。以数字方式表示,默认值“:80”
    @note: [可省略]
    */
    unsigned int port = 0;

    /** 路径。以“/”字符区别路径中的每一个目录名称
    @note: [可省略]
    */
    std::string path;

    /** 查询。GET模式的窗体参数，以“?”字符为起点，每个参数以“&”隔开，再以“=”分开参数名称与数据，通常以UTF8的URL编码，避开字符冲突的问题
    @note: [可省略]
    */
    std::string query;

    /** 片段。以“#”字符为起点
    @note: [可省略]
    */
    std::string fragment;

    /** url 是否有效
    @return 是否有效
    */
    bool is_valid() {
        return !scheme.empty() && port != 0 && !host.empty();
    }

    std::string uri()
    {
        std::string uri = "";

        if (!is_valid()) {
            return uri;
        }

        if (path.length() > 0) {
            uri += path;
        }

        if (query.length() > 0) {
            uri += "?";
            uri += query;
        }

        if (fragment.length() > 0) {
            uri += "#";
            uri += fragment;
        }

        return uri;
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

        formatedUrl += scheme;
        formatedUrl += "://";

        if (user.length() != 0 || password.length() != 0) {
            formatedUrl += user;

            if (password.length() != 0) {
                formatedUrl += ":";
                formatedUrl += password;
            }
            
            formatedUrl += "@";
        }

        formatedUrl += host;

        if (!( (port == 80 || port == 443) &&
               (scheme.length() >= 4) &&
               (scheme[0] == 'h' || scheme[0] == 'H') &&
               (scheme[1] == 't' || scheme[1] == 'T') &&
               (scheme[2] == 't' || scheme[2] == 'T') &&
               (scheme[3] == 'p' || scheme[3] == 'P') )) {
            formatedUrl += ":";
            char portBuff[10] = {0};
            ::_itoa_s(port, portBuff, 10);
            formatedUrl += portBuff;
        }

        if (path.length() > 0) {
            formatedUrl += path;
        }

        if (query.length() > 0) {
            formatedUrl += "?";
            formatedUrl += query;
        }

        if (fragment.length() > 0) {
            formatedUrl += "#";
            formatedUrl += fragment;
        }

        return formatedUrl;
    }
};

/** 解析url
@note: 是否成功用ddurl::is_valid 判断
@param [in] url_str 原始字符串（一般以UTF-8编码）
@param [out] url 输出
*/
inline void parse_url(const std::string& url_str, ddurl& url)
{
    size_t index = 0;
    size_t len = url_str.length();

    // 找scheme
    for (size_t i = index; i < len; ++i) {
        if (url_str[i] == ':') {
            url.scheme = url_str.substr(index, i - index);
            index = i + 1;
            break;
        }
    }

    if (url.scheme.empty()) {
        return;
    }

    // 固定 // 或者 '\\'
    if (index + 1 >= len ||
        (url_str[index] != '/' && url_str[index] != '\\') ||
        (url_str[index + 1] != '/' && url_str[index + 1] != '\\')) {
        return;
    }

    // 指向 “/”
    index += 1;

    // [user[:password]@]host[:port] 【[/path] [?query] [#fragment]】
    // 找到扩展符号 “/ \ ? #”
    size_t hostPortEndIndex = len - 1;
    for (size_t i = index + 1; i < len; ++i) {
        if (url_str[i] == '/' || url_str[i] == '\\' || url_str[i] == '?' || url_str[i] == '#') {
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
            if (url_str[i] == '@') {
                userPswEndIndex = i - 1;

                // 长度不为0
                if (userPswEndIndex > index) {
                    // 寻找 “:”
                    size_t userEndIndex = userPswEndIndex;
                    for (size_t j = index + 1; j <= userPswEndIndex; ++j) {
                        if (url_str[j] == ':') {
                            userEndIndex = j - 1;
                            break;
                        }
                    }

                    url.user = url_str.substr(index + 1, userEndIndex - index);

                    if (userEndIndex < userPswEndIndex - 2) {
                        url.password = url_str.substr(userEndIndex + 2, userPswEndIndex - userEndIndex - 1);
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
                if (url_str[j] == ':') {
                    hostEndIndex = j - 1;
                    break;
                }
            }

            // 长度为0
            if (hostEndIndex < index) {
                return;
            }

            url.host = url_str.substr(index + 1, hostEndIndex - index);

            if (hostEndIndex < hostPortEndIndex - 2) {
                url.port = ::atoi(url_str.substr(hostEndIndex + 2, hostPortEndIndex - hostEndIndex - 1).c_str());
            }

            if (url.port == 0) {
                if (url.scheme.size() == 5 &&
                    (url.scheme[0] == 'h' || url.scheme[0] == 'H') &&
                    (url.scheme[1] == 't' || url.scheme[1] == 'T') &&
                    (url.scheme[2] == 't' || url.scheme[2] == 'T') &&
                    (url.scheme[3] == 'p' || url.scheme[3] == 'P') &&
                    (url.scheme[4] == 's' || url.scheme[4] == 'S') ) {
                    url.port = 443;
                } else {
                    url.port = 80;
                }
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
            if (url_str[i] == '?' || url_str[i] == '#' || url_str[i] == '/' || url_str[i] == '\\') {
                extendEndIndex = i - 1;
                break;
            }
        }

        // 如果上一个符号是 "/" 或者 "\\" 那么填充填充path
        if (url_str[index] == '/' || url_str[index] == '\\') {
            // 继续寻找直到 ? #
            size_t i = extendEndIndex;
            extendEndIndex = len - 1;
            for (; i < len; ++i) {
                if (url_str[i] == '?' || url_str[i] == '#') {
                    extendEndIndex = i - 1;
                    break;
                }
            }

            url.path = url_str.substr(index, extendEndIndex - index + 1);
            size_t pathEndIndex = 0;

            for (size_t ii = 0; ii < url.path.length(); ++ii) {
                if (url.path[ii] == '\\' || url.path[ii] == '/') {
                    if (pathEndIndex == 0 || url.path[pathEndIndex - 1] != '/') {
                        url.path[pathEndIndex++] = '/';
                    }
                } else {
                    url.path[pathEndIndex++] = url.path[ii];
                }
            }

            url.path[pathEndIndex] = 0;
            url.path = url.path.c_str();
        } else if (url_str[index] == '?') {
            // 如果上一个符号是 "?" 那么填充query
            url.query = url_str.substr(index + 1, extendEndIndex - index);
        } else if (url_str[index] == '#') {
            // 如果上一个符号是 "#" 那么填充fragment
            url.fragment = url_str.substr(index + 1, extendEndIndex - index);
        }

        index = extendEndIndex + 1;
    }
}

/** 正则表达式解析url,和parse_url稍有不同parse_urlRegex对[/path] [?query] [#fragment]的顺序严格要求
scheme://[user[:password]@]host[:port] [/path] [?query] [#fragment]
@note: 是否成功用dddurl::is_valid 判断
@param [in] url_str原始字符串（一般以UTF-8编码）
@param [out] url 输出
*/
inline void parse_url_regex(const std::string& url_str, ddurl& url)
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

    if (!std::regex_search(url_str, results, partten) || results.size() != 9) {
        return;
    }

    url.scheme = results[1];
    url.user = results[2];
    url.password = results[3];
    url.host = results[4];
    url.port = ::atoi(results[5].str().c_str());
    url.path = results[6];
    url.query = results[7];
    url.fragment = results[8];

    size_t pathEndIndex = 0;

    for (size_t i = 0; i < url.path.length(); ++i) {
        if (url.path[i] == '\\' || url.path[i] == '/') {
            if (pathEndIndex == 0 || url.path[pathEndIndex - 1] != '/') {
                url.path[pathEndIndex++] = '/';
            }
        } else {
            url.path[pathEndIndex++] = url.path[i];
        }
    }

    url.path[pathEndIndex] = 0;
    url.path = url.path.c_str();

    if (url.port == 0) {
        if (url.scheme.size() == 5 &&
            (url.scheme[0] == 'h' || url.scheme[0] == 'H') &&
            (url.scheme[1] == 't' || url.scheme[1] == 'T') &&
            (url.scheme[2] == 't' || url.scheme[2] == 'T') &&
            (url.scheme[3] == 'p' || url.scheme[3] == 'P') &&
            (url.scheme[4] == 's' || url.scheme[4] == 's') ) {
            url.port = 443;
        } else {
            url.port = 80;
        }
    }
}

} // namespace NSP_DD
#endif // ddbase_str_ddurl_hpp_
