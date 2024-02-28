#ifndef ddbase_str_ddjson_h_
#define ddbase_str_ddjson_h_

#include "ddbase/dddef.h"
#include "ddbase/ddassert.h"
#include <vector>
#include <string>
#include <map>
#include <stack>

namespace NSP_DD {
// tmp
class ddjson_value
{
public:
    enum class ddtype {
        null, boolean, number_s64, number_double, str, array, object
    };

    ddjson_value() = default;

    ddjson_value(ddtype type)
    {
        m_type = type;
        if (is_str()) {
            m_value.str = new std::string();
        } else if (is_array()) {
            m_value.arr = new std::vector<ddjson_value>();
        } else if (is_obj()) {
            m_value.obj = new std::map<std::string, ddjson_value>();
        }
    }

    ddjson_value(const ddjson_value& r)
    {
        m_type = r.type();
        if (is_str()) {
            m_value.str = new std::string(r.as_str());
        } else if (is_array()) {
            m_value.arr = new std::vector<ddjson_value>(r.as_array());
        } else if (is_obj()) {
            m_value.obj = new std::map<std::string, ddjson_value>(r.as_obj());
        } else {
            m_value = r.m_value;
        }
    }

    ddjson_value& operator=(const ddjson_value& r)
    {
        ddjson_value tmp(r);
        swap(*this, tmp);
        return *this;
    }

    ddjson_value(ddjson_value&& r) noexcept
    {
        swap(*this, r);
    }

    ddjson_value& operator=(ddjson_value&& r) noexcept
    {
        swap(*this, r);
        return *this;
    }

    ~ddjson_value()
    {
        if (is_str()) {
            delete m_value.str;
        } else if (is_array() || is_obj()) {
            std::stack<ddjson_value> jvs;
            jvs.push(ddjson_value(std::move(*this)));
            while (!jvs.empty()) {
                ddjson_value& top = jvs.top();
                if (top.is_array()) {
                    for (auto& it : top.as_array()) {
                        jvs.push(ddjson_value(std::move(it)));
                    }
                    delete top.m_value.arr;
                } else if (top.is_obj()) {
                    for (auto& it : top.as_obj()) {
                        jvs.push(ddjson_value(std::move(it.second)));
                    }
                    delete top.m_value.obj;
                } else if (top.is_str()) {
                    delete top.m_value.str;
                }
                top.m_type = ddtype::null;
                jvs.pop();
            }
        }
    }

    inline const ddtype& type() const { return m_type; }
    inline bool is_null() const { return type() == ddtype::null; }

    inline bool is_boolean() const { return type() == ddtype::boolean; }
    inline const bool& as_boolean() const { DDASSERT(is_boolean()); return m_value.b; }
    inline bool& as_boolean() { DDASSERT(is_boolean()); return m_value.b; }

    inline bool is_s64() const { return type() == ddtype::number_s64; }
    inline const s64& as_s64() const { DDASSERT(is_s64()); return m_value.num_s64; }
    inline s64& as_s64() { DDASSERT(is_s64()); return m_value.num_s64; }

    inline bool is_double() const { return type() == ddtype::number_double; }
    inline const double& as_double() const { DDASSERT(is_double()); return m_value.num_double; }
    inline double& as_double() { DDASSERT(is_double()); return m_value.num_double; }

    inline bool is_str() const { return type() == ddtype::str; }
    inline const std::string& as_str() const { DDASSERT(is_str()); DDASSERT(m_value.str != nullptr); return *m_value.str; }
    inline std::string& as_str() { DDASSERT(is_str()); DDASSERT(m_value.str != nullptr); return *m_value.str; }

    inline bool is_array() const { return type() == ddtype::array; }
    inline const std::vector<ddjson_value>& as_array() const { DDASSERT(is_array()); DDASSERT(m_value.arr != nullptr); return *m_value.arr; }
    inline std::vector<ddjson_value>& as_array() { DDASSERT(is_array()); DDASSERT(m_value.arr != nullptr); return *m_value.arr; }

    inline bool is_obj() const { return type() == ddtype::object; }
    inline const std::map<std::string, ddjson_value>& as_obj() const { DDASSERT(is_obj()); DDASSERT(m_value.obj != nullptr); return *m_value.obj; }
    inline std::map<std::string, ddjson_value>& as_obj() { DDASSERT(is_obj()); DDASSERT(m_value.obj != nullptr); return *m_value.obj; }

private:
    ddtype m_type = ddtype::null;
    union {
        bool b;
        s64 num_s64;
        double num_double;
        std::string* str;
        std::vector<ddjson_value>* arr;
        std::map<std::string, ddjson_value>* obj;
    } m_value{};

    friend void swap(ddjson_value& l, ddjson_value& r) noexcept
    {
        std::swap(l.m_type, r.m_type);
        std::swap(l.m_value, r.m_value);
    }
};

using ddjv = ddjson_value;
using ddjvt = ddjson_value::ddtype;

class ddjson
{
public:
    static std::string to_string(const ddjv& jv);
    struct ddparse_result
    {
        bool successful = true;
        std::string error_message;
        s32 pos = 0;
        ddjv jv;
    };
    inline static ddparse_result parse(const std::string& json_str)
    {
        return ddjson_parser(json_str).parse();
    }

private:
    class ddjson_parser
    {
    public:
        ddjson_parser(const std::string& json_str) :
            m_str(json_str)
        {
            m_str_len = (s32)m_str.length();
        }

        ddparse_result parse()
        {
            ddparse_result result;
            parse_x(result.jv);
            result.successful = m_successful;
            result.error_message = m_error_message;
            result.pos = m_pos;
            return result;
        }

    private:
        // before call each parse_xx function should make sure the m_pos point to the first not blank char
        // each parse_xx function will move m_pos
        struct ddparse_ctx;
        enum class ddparse_event
        {
            init,
            parse,
            set_sub_item
        };
        typedef void (ddjson_parser::* action_fn)(ddparse_event event, ddparse_ctx* ctx);
        struct ddparse_ctx {
            bool end = false;
            action_fn action = nullptr;
            ddjv sub_jv;
            std::string key; // for parse_object using
            ddjv jv;
        };

        void parse_x(ddjv& jv)
        {
            parse_blank();
            char c = get();
            if (c == '{') {
                parse_object(ddparse_event::init, nullptr);
            } else if (c == '[') {
                parse_array(ddparse_event::init, nullptr);
            } else {
                m_successful = false;
                m_error_message = "A JSON payload should be an object or array, not a string.";
                return;
            }

            while (!m_stacks.empty()) {
                auto current = m_stacks.top();
                (this->*current->action)(ddparse_event::parse, current);
                if (current->end || !m_successful) {
                    if (m_successful) {
                        m_stacks.pop();
                        if (!m_stacks.empty()) {
                            auto next = m_stacks.top();
                            next->sub_jv = std::move(current->jv);
                            (this->*next->action)(ddparse_event::set_sub_item, next);
                        } else {
                            jv = std::move(current->jv);
                        }
                        delete current;
                    } else {
                        break;
                    }
                }
            }

            // clean
            while (!m_stacks.empty()) {
                auto current = m_stacks.top();
                m_stacks.pop();
                delete current;
            }

            if (m_successful) {
                parse_blank();
                if (m_successful && m_pos != m_str.length()) {
                    m_successful = false;
                    m_error_message = "Redundant invalid characters.";
                }
            }
        }

        void parse_array(ddparse_event event, ddparse_ctx* ctx)
        {
            if (event == ddparse_event::init) {
                if (get() != '[') {
                    m_successful = false;
                    m_error_message = "Miss: '['";
                    return;
                }
                m_pos++; // next of '['

                DDASSERT(ctx == nullptr);
                ddparse_ctx* new_ctx = new ddparse_ctx();
                new_ctx->jv = ddjv(ddjvt::array);
                new_ctx->action = &ddjson_parser::parse_array;
                m_stacks.push(new_ctx);
            } else if (event == ddparse_event::parse) {
                if (m_pos >= m_str_len) {
                    m_successful = false;
                    m_error_message = "Miss: ']'";
                    return;
                }

                parse_blank();
                char c = m_str[m_pos];
                if (c == ']') {
                    ++m_pos;
                    parse_blank();
                    parse_end_comma();
                    ctx->end = true;
                    return;
                }

                if (c == '[') {
                    parse_array(ddparse_event::init, nullptr);
                } else if (c == '{') {
                    parse_object(ddparse_event::init, nullptr);
                } else {
                    ddjv sub_jv;
                    parse_value(sub_jv);
                    if (m_successful) {
                        ctx->sub_jv = std::move(sub_jv);
                        parse_array(ddparse_event::set_sub_item, ctx);
                    }
                }
            } else if (event == ddparse_event::set_sub_item) {
                ctx->jv.as_array().push_back(std::move(ctx->sub_jv));
            }
        }

        // "{xxx},"
        void parse_object(ddparse_event event, ddparse_ctx* ctx)
        {
            if (event == ddparse_event::init) {
                if (get() != '{') {
                    m_successful = false;
                    m_error_message = "Miss: '{'.";
                    return;
                }

                m_pos++; // next of '{'

                DDASSERT(ctx == nullptr);
                ddparse_ctx* new_ctx = new ddparse_ctx();
                new_ctx->jv = ddjv(ddjvt::object);
                new_ctx->action = &ddjson_parser::parse_object;
                m_stacks.push(new_ctx);
            } else if (event == ddparse_event::parse) {
                if (m_pos >= m_str_len) {
                    m_successful = false;
                    m_error_message = "Miss: '}'";
                    return;
                }

                parse_blank();
                char c = m_str[m_pos];
                if (c == '}') {
                    ++m_pos;
                    parse_blank();
                    parse_end_comma();
                    ctx->end = true;
                    return;
                }

                ctx->key.clear();
                parse_key(ctx->key);
                if (!m_successful) {
                    return;
                }

                parse_blank();
                c = m_str[m_pos];
                if (c == '[') {
                    parse_array(ddparse_event::init, nullptr);
                } else if (c == '{') {
                    parse_object(ddparse_event::init, nullptr);
                } else {
                    ddjv sub_jv;
                    parse_value(sub_jv);
                    if (m_successful) {
                        ctx->sub_jv = std::move(sub_jv);
                        parse_object(ddparse_event::set_sub_item, ctx);
                    }
                }
            } else if (event == ddparse_event::set_sub_item) {
                ctx->jv.as_obj()[ctx->key] = (std::move(ctx->sub_jv));
            }
        }

        void parse_value(ddjv& jv)
        {
            char c = get();
            if (c == '"') {
                std::string formatted;
                parse_string(formatted);
                if (m_successful) {
                    jv = ddjv(ddjvt::str);
                    jv.as_str() = formatted;
                    parse_blank();
                    parse_end_comma();
                }
                return;
            }

            std::string value_str;
            for (auto i = m_pos; i < m_str_len; ++i) {
                c = m_str[i];
                if (c == ',' || c == ']' || c == '}' || c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                    value_str = m_str.substr(m_pos, i - m_pos);
                    break;
                }
            }

            if (value_str.length() == 0) {
                m_successful = false;
                m_error_message = "Invalid json value";
                return;
            }

            if (value_str == "null") {
                jv = ddjv(ddjvt::null);
            } else if (value_str == "true") {
                jv = ddjv(ddjvt::boolean);
                jv.as_boolean() = true;
            } else if (value_str == "false") {
                jv = ddjv(ddjvt::boolean);
                jv.as_boolean() = false;
            } else {
                // number
                s64 int_number = 0;
                double double_number = 0.0;
                bool is_double = false;
                bool is_overflow = false;
                s32 number_parsed_pos = ddjson_math_utils::from_json_number_str(value_str, int_number, double_number, is_double, is_overflow);
                if (number_parsed_pos == value_str.length()) {
                    if (is_double) {
                        jv = ddjv(ddjvt::number_double);
                        jv.as_double() = double_number;
                    } else {
                        jv = ddjv(ddjvt::number_s64);
                        jv.as_s64() = int_number;
                    }
                } else {
                    m_error_message = "Invalid json value: ";
                    m_error_message += value_str;
                    m_successful = false;
                    return;
                }
            }

            m_pos += (s32)value_str.length();
            parse_blank();
            parse_end_comma();
        }

        // parse key until ":" such as "key":"value", the result.pos will point at the next char of ":"
        inline void parse_key(std::string& key)
        {
            parse_string(key);
            if (!m_successful) {
                return;
            }

            parse_blank();
            if (get() != ':') {
                m_error_message = "Miss: ':'";
                m_successful = false;
                return;
            }
            ++m_pos;
        }

        inline void parse_string(std::string& parsed)
        {
            if (get() != '"') {
                m_error_message = "Miss begin: '\"'";
                m_successful = false;
                return;
            }

            parsed.clear();
            ++m_pos; // // next of '"'
            while (m_pos < m_str_len) {
                u8 c = (u8)m_str[m_pos];
                if (c <= 0x1F) {
                    m_error_message = "control characters are not allowed such as '\\r' '\\n'";
                    m_successful = false;
                    return;
                } else if (c == '"') {
                    // find the end of '"'
                    ++m_pos;
                    return;
                } else if (c == '\\') {
                    if (m_pos + 1 >= m_str_len) {
                        m_error_message = "single '\' is not allowed";
                        m_successful = false;
                        return;
                    }
                    char next_char = m_str[m_pos + 1];
                    switch (next_char) {
                    case '\"':
                        m_pos += 2;
                        parsed.push_back('\"');
                        break;
                    case '\\':
                        m_pos += 2;
                        parsed.push_back('\\');
                        break;
                    case '/':
                        m_pos += 2;
                        parsed.push_back('/');
                        break;
                    case 'b':
                        m_pos += 2;
                        parsed.push_back('\b');
                        break;
                    case 'f':
                        m_pos += 2;
                        parsed.push_back('\f');
                        break;
                    case 'r':
                        m_pos += 2;
                        parsed.push_back('\r');
                        break;
                    case 'n':
                        m_pos += 2;
                        parsed.push_back('\n');
                        break;
                    case 't':
                        m_pos += 2;
                        parsed.push_back('\t');
                        break;
                    case 'u':
                    {
                        int codepoint = parse_codepoint();
                        if (codepoint == -1) {
                            m_error_message = "unicode format error: '\\u' must be followed by 4 hex digits";
                            m_successful = false;
                            return;
                        }

                        if (0xDC00 <= codepoint && codepoint <= 0xDFFF) {
                            m_error_message = "unicode format error: ~[0xDC00,0xDFFF]";
                            m_successful = false;
                            return;
                        }

                        // if code point is a high surrogate
                        if (0xD800 <= codepoint && codepoint <= 0xDBFF) {
                            const int codepoint2 = parse_codepoint();
                            if (codepoint2 == -1 || !(0xDC00 <= codepoint2 && codepoint2 <= 0xDFFF)) {
                                m_error_message = "unicode format error: [0xD800,0xDBFF][0xDC00,0xDFFF]";
                                m_successful = false;
                                return;
                            }
                            int codepoint1 = codepoint;
                            codepoint = static_cast<int>(
                                // high surrogate occupies the most significant 22 bits
                                (static_cast<unsigned int>(codepoint1) << 10u)
                                // low surrogate occupies the least significant 15 bits
                                + static_cast<unsigned int>(codepoint2)
                                // there is still the 0xD800, 0xDC00 and 0x10000 noise
                                // in the result, so we have to subtract with:
                                // (0xD800 << 10) + DC00 - 0x10000 = 0x35FDC00
                                -0x35FDC00u);
                        }

                        DDASSERT(0x00 <= codepoint && codepoint <= 0x10FFFF);
                        // translate codepoint into bytes
                        if (codepoint < 0x80) {
                            // 1-byte characters: 0xxxxxxx (ASCII)
                            parsed.push_back(char(codepoint));
                        } else if (codepoint <= 0x7FF) {
                            // 2-byte characters: 110xxxxx 10xxxxxx
                            parsed.push_back(char(0xC0u | (static_cast<unsigned int>(codepoint) >> 6u)));
                            parsed.push_back(char(0x80u | (static_cast<unsigned int>(codepoint) & 0x3Fu)));
                        } else if (codepoint <= 0xFFFF) {
                            // 3-byte characters: 1110xxxx 10xxxxxx 10xxxxxx
                            parsed.push_back(char(0xE0u | (static_cast<unsigned int>(codepoint) >> 12u)));
                            parsed.push_back(char(0x80u | ((static_cast<unsigned int>(codepoint) >> 6u) & 0x3Fu)));
                            parsed.push_back(char(0x80u | (static_cast<unsigned int>(codepoint) & 0x3Fu)));
                        } else {
                            // 4-byte characters: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                            parsed.push_back(char(0xF0u | (static_cast<unsigned int>(codepoint) >> 18u)));
                            parsed.push_back(char(0x80u | ((static_cast<unsigned int>(codepoint) >> 12u) & 0x3Fu)));
                            parsed.push_back(char(0x80u | ((static_cast<unsigned int>(codepoint) >> 6u) & 0x3Fu)));
                            parsed.push_back(char(0x80u | (static_cast<unsigned int>(codepoint) & 0x3Fu)));
                        }
                        break;
                    }
                    default:
                        m_successful = false;
                        m_error_message = "Invalid char after \\";
                        return;
                    }
                } else {
                    // is well utf-8 format
                    auto parsed_utf8_code_fn = [](
                        s32& i,
                        const std::string& src,
                        const std::vector<u8>& ranges,
                        std::string& parsed
                        ) {
                            if (i + ranges.size() / 2 >= src.length()) {
                                return false;
                            }

                            for (auto j = 0; j < ranges.size(); j += 2) {
                                u8 c = (u8)src[i + 1 + j / 2];
                                if (!(c >= ranges[j] && c <= ranges[j + 1])) {
                                    return false;
                                }
                            }

                            parsed.push_back(src[i++]);
                            for (auto j = 0; j < ranges.size(); j += 2) {
                                parsed.push_back(src[i++]);
                            }
                            return true;
                        };

                    if (c >= 0x20 && c <= 0x7F) {
                        parsed.push_back(m_str[m_pos++]);
                    } else if (c >= 0xC2 && c <= 0xDF) {
                        if (!parsed_utf8_code_fn(m_pos, m_str, { 0x80, 0xBF }, parsed)) {
                            m_error_message = "utf8 format error: [0xC2,0xDF][0x80,0xBF]";
                            m_successful = false;
                            return;
                        }
                    } else if (c == 0xE0) {
                        if (!parsed_utf8_code_fn(m_pos, m_str, { 0xA0, 0xBF, 0x80, 0xBF }, parsed)) {
                            m_error_message = "utf8 format error: [0xE0][0xA0,0xBF][0x80,0xBF]";
                            m_successful = false;
                            return;
                        }
                    } else if ((c >= 0xE1 && c <= 0xEC) || (c >= 0xEE && c <= 0xEF)) {
                        if (!parsed_utf8_code_fn(m_pos, m_str, { 0x80, 0xBF, 0x80, 0xBF }, parsed)) {
                            m_error_message = "utf8 format error: [0xE1,0xEC/0xEE,0xEF][0x80, 0xBF][0x80,0xBF]";
                            m_successful = false;
                            return;
                        }
                    } else if (c == 0xED) {
                        if (!parsed_utf8_code_fn(m_pos, m_str, { 0x80, 0x9F, 0x80, 0xBF }, parsed)) {
                            m_error_message = "utf8 format error: [0xED][0x80,0x9F][0x80,0xBF]";
                            m_successful = false;
                            return;
                        }
                    } else if (c == 0xF0) {
                        if (!parsed_utf8_code_fn(m_pos, m_str, { 0x90, 0xBF, 0x80, 0xBF, 0x80, 0xBF }, parsed)) {
                            m_error_message = "utf8 format error: [0xF0][0x90,0xBF][0x80,0xBF][0x80,0xBF]";
                            m_successful = false;
                            return;
                        }
                    } else if (c >= 0xF1 && c <= 0xF3) {
                        if (!parsed_utf8_code_fn(m_pos, m_str, { 0x80, 0xBF, 0x80, 0xBF, 0x80, 0xBF }, parsed)) {
                            m_error_message = "utf8 format error: [0xF0][0x80,0xBF][0x80,0xBF][0x80,0xBF]";
                            m_successful = false;
                            return;
                        }
                    } else if (c == 0xF4) {
                        if (!parsed_utf8_code_fn(m_pos, m_str, { 0x80, 0x8F, 0x80, 0xBF, 0x80, 0xBF }, parsed)) {
                            m_error_message = "utf8 format error: [0xF0][0x80,0x8F][0x80,0xBF][0x80,0xBF]";
                            m_successful = false;
                            return;
                        }
                    } else {
                        m_error_message = "utf8 format error";
                        m_successful = false;
                        return;
                    }
                }
            }
        }

        // "\u c1 c2 c3 c4" the codepoint is:
        //   (c1 * 0x1000) + (c2 * 0x0100) + (c3 * 0x0010) + c4
        // = (c1 << 12) + (c2 << 8) + (c3 << 4) + (c4 << 0)
        // return -1 if not valid
        inline int parse_codepoint()
        {
            // \uXXXX
            if (m_pos + 5 >= m_str.length() || m_str[m_pos] != '\\' || m_str[m_pos + 1] != 'u') {
                return -1;
            }

            int codepoint = 0;
            const u32 factors[4] = { 12u, 8u, 4u, 0u };
            for (s32 i = 0; i < 4; ++i) {
                char c = m_str[m_pos + i + 2];
                u32 factor = factors[i];
                if (c >= '0' && c <= '9') {
                    codepoint += static_cast<int>((static_cast<unsigned int>(c) - 0x30u) << factor);
                } else if (c >= 'A' && c <= 'F') {
                    codepoint += static_cast<int>((static_cast<unsigned int>(c) - 0x37u) << factor);
                } else if (c >= 'a' && c <= 'f') {
                    codepoint += static_cast<int>((static_cast<unsigned int>(c) - 0x57u) << factor);
                } else {
                    return -1;
                }
            }

            DDASSERT(0x0000 <= codepoint && codepoint <= 0xFFFF);
            m_pos += 6;
            return codepoint;
        }

        // the m_pos will point to the next char of ',' or ']' or '}'
        inline void parse_end_comma()
        {
            if (m_pos >= m_str_len) {
                return;
            }

            char c = get();
            if (c == ',') {
                ++m_pos;
                parse_blank();
                if (m_pos >= m_str_len) {
                    m_successful = false;
                    m_error_message = "Extra comma";
                }
                c = get();
                if (c == ']' || c == '}') {
                    m_successful = false;
                    m_error_message = "Extra comma";
                }
                return;
            }

            if (c == ']' || c == '}') {
                return;
            }

            m_error_message = "Miss: ','";
            m_successful = false;
        }

        // the m_pos will point to the next char of ' ' or '\t' or '\r' or '\n'
        inline void parse_blank()
        {
            while (char c = get()) {
                if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                    break;
                }
                ++m_pos;
            }
        }

        inline u8 get()
        {
            if (m_pos >= m_str_len) {
                return 0;
            }
            return m_str[m_pos];
        }

        std::stack<ddparse_ctx*> m_stacks;
        const std::string& m_str;
        s32 m_str_len = 0;
        bool m_successful = true;
        std::string m_error_message;
        s32 m_pos = 0;
        s32 m_deep = 0;
    };

    class ddjson_math_utils
    {
    public:
        static inline s32 from_json_number_str(const std::string& str, s64& int_number, double& double_number, bool& is_double, bool& overflow)
        {
            overflow = false;
            const s32 len = (s32)str.length();
            s32 frac_pos = len;
            s32 exp_pos = len;
            for (auto i = 0; i < str.length(); ++i) {
                char c = str[i];
                if (c == '.') {
                    if (frac_pos == len) {
                        frac_pos = i;
                    } else {
                        return i;
                    }
                } else if (c == 'e' || c == 'E') {
                    if (exp_pos == len) {
                        exp_pos = i;
                    } else {
                        return i;
                    }
                }
            }

            if (exp_pos != len && frac_pos != len) {
                if (exp_pos < frac_pos || exp_pos == frac_pos + 1) {
                    return exp_pos;
                }
            }

            if (frac_pos == 0 || frac_pos == len - 1) {
                return frac_pos;
            }

            if (exp_pos == 0 || exp_pos == len - 1) {
                return exp_pos;
            }

            // parse int part
            s64 int_part = 0;
            bool int_part_overflow = false;
            s32 int_part_len = frac_pos;
            if (frac_pos == len && exp_pos != len) {
                int_part_len = exp_pos;
            }
            s32 pos = to_s64(str.c_str(), int_part_len, true, false, int_part, int_part_overflow);
            if (pos != int_part_len) {
                return pos;
            }

            double double_part = 0.0;
            bool double_part_overflow = false;
            if (frac_pos != len) {
                pos = to_double(str.c_str() + frac_pos + 1, exp_pos - frac_pos - 1, double_part, double_part_overflow);
                if (pos != exp_pos - frac_pos - 1) {
                    return pos + frac_pos + 1;
                }
            }

            s64 exp = 0;
            if (exp_pos != len) {
                bool dummy = false;
                pos = to_s64(str.c_str() + exp_pos + 1, len - exp_pos - 1, false, true, exp, dummy);
                if (pos != len - exp_pos - 1) {
                    return pos + exp_pos + 1;
                }
            }

            if (int_part_overflow || double_part_overflow) {
                overflow = true;
                return len;
            }

            if (frac_pos != len) {
                is_double = true;
                double_number = (int_part + double_part) * std::pow(10.0, exp);
            } else {
                int_number = int_part * (s64)std::pow(10, exp);
            }
            overflow = false;
            return len;
        }

        static s32 to_s64(const char* str, s32 len, bool check_first_0, bool allow_plus_sign, s64& result, bool& overflow)
        {
            DDASSERT(len > 0);
            overflow = false;
            result = 0;
            bool is_negative = false;
            s32 pos = 0;
            if (str[pos] == '-') {
                if (len == 1) {
                    return 0;
                }
                is_negative = true;
                ++pos;
            } else if (allow_plus_sign && str[pos] == '+') {
                if (len == 1) {
                    return 0;
                }
                ++pos;
            }

            if (check_first_0) {
                if (str[pos] == '0') {
                    ++pos;
                    return pos;
                }
            }

            for (; pos < len; ++pos) {
                char c = str[pos];
                if (c < '0' || c > '9') {
                    return pos;
                }

                result = result * 10 + (c - '0');
            }

            if (is_negative) {
                result *= -1;
            }
            return pos;
        }

        static s32 to_double(const char* str, s32 len, double& result, bool& overflow)
        {
            DDASSERT(len > 0);
            overflow = false;
            result = 0.0;
            double base = 0.1;
            for (s32 pos = 0; pos < len; ++pos) {
                char c = str[pos];
                if (c < '0' || c > '9') {
                    return pos;
                }

                result = result + (c - '0') * base;
                base *= 0.1;
            }
            return len;
        }
    };
};
} // namespace NSP_DD
#endif // ddbase_str_ddjson_h_
