#include "ddbase/stdafx.h"
#include "ddbase/file/ddpath.h"
#include "ddbase/str/ddstr.h"
namespace NSP_DD {
std::wstring ddpath::name(const std::wstring& path)
{
    size_t idx = path.find_last_of(L'\\');
    if (idx == std::wstring::npos) {
        idx = path.find_last_of(L'/');
        if (idx == std::wstring::npos) {
            return L"";
        }
    }

    return path.substr(idx + 1);
}

void ddpath::normal(std::wstring& path, wchar_t separator /* = ddpath_separator */)
{
    size_t index = 0;
    for (size_t i = 0; i < path.size(); ++i) {
        if (path[i] == L'\\' || path[i] == L'/') {
            if (index >= 1 && path[index - 1] == separator) {
                continue;
            }
            path[index++] = separator;
        } else {
            path[index++] = path[i];
        }
    }

    if (index >= 1 && path[index - 1] == separator) {
        --index;
    }
    path.resize(index);
}

std::wstring ddpath::normal1(const std::wstring& path, wchar_t separator /* = ddpath_separator */)
{
    std::wstring normal_path = path;
    normal(normal_path, separator);
    return normal_path;
}

void ddpath::expand(std::wstring& path, wchar_t separator)
{
    normal(path, separator);
    std::vector<std::wstring> path_vec;
    ddstr::split(path.c_str(), std::wstring(1, separator).c_str(), path_vec);
    std::stack<std::wstring> path_stack;
    for (const auto& it : path_vec) {
        if (it == L".") {
            continue;
        }

        if (it == L"..") {
            if (!path_stack.empty() && path_stack.top() != L"..") {
                path_stack.pop();
                continue;
            }
        }
        path_stack.push(it);
    }
    path = L"";
    while (!path_stack.empty()) {
        path = path_stack.top() + separator + path;
        path_stack.pop();
    }
    if (!path.empty()) {
        path.resize(path.size() - 1);
    }
}

std::wstring ddpath::expand1(const std::wstring& path, wchar_t separator)
{
    std::wstring normal_path = path;
    expand(normal_path, separator);
    return normal_path;
}

std::wstring ddpath::join(const std::wstring& l, const std::wstring& r, wchar_t separator /* = ddpath_separator */)
{
    if (r.empty()) {
        return l;
    }
    if (l.empty()) {
        return r;
    }

    std::wstring join_str = l;
    if (l[l.length() - 1] == L'\\' || l[l.length() - 1] == L'/') {
        join_str.resize(l.length() - 1);
    }
    join_str += separator;
    if (r[0] == L'\\' || r[0] == L'/') {
        join_str += (r.c_str() + 1);
    } else {
        join_str += r.c_str();
    }
    return join_str;
}

std::wstring ddpath::join(const std::vector<std::wstring>& l, wchar_t separator)
{
    std::wstring join_path;
    for (const auto& it : l) {
        join_path = join(join_path, it, separator);
    }
    return join_path;
}

std::wstring ddpath::suffix(const std::wstring& path)
{
    std::wstring path_name = name(path);
    size_t idx = path_name.find_last_of(L'.');
    if (idx == std::wstring::npos) {
        return L"";
    }

    return path_name.substr(idx + 1);
}

std::wstring ddpath::parent(const std::wstring& path)
{
    size_t idx = path.find_last_of(L'\\');
    if (idx == std::wstring::npos) {
        idx = path.find_last_of(L'/');
        if (idx == std::wstring::npos) {
            return L"";
        }
    }

    return path.substr(0, idx);
}
std::wstring ddpath::relative_path(const std::wstring& base_full_path, const std::wstring& src_full_path, wchar_t separator /*= ddpath_separator*/)
{
    std::vector<std::wstring> base_path_vec;
    ddstr::split(ddpath::expand1(base_full_path, separator).c_str(), std::wstring(1, separator).c_str(), base_path_vec);
    std::vector<std::wstring> src_path_vec;
    ddstr::split(ddpath::expand1(src_full_path, separator).c_str(), std::wstring(1, separator).c_str(), src_path_vec);

    size_t i = 0;
    for (; i < base_path_vec.size() && i < src_path_vec.size(); ++i) {
        if (base_path_vec[i] != src_path_vec[i]) {
            break;
        }
    }
    std::wstring relative_path;
    for (size_t j = i; j < base_path_vec.size(); ++j) {
        relative_path += L"..";
        relative_path += separator;
    }
    for (size_t j = i; j < src_path_vec.size(); ++j) {
        relative_path += src_path_vec[j];
        relative_path += separator;
    }

    if (!relative_path.empty()) {
        relative_path.resize(relative_path.size() - 1);
    }
    return relative_path;
}
} // namespace NSP_DD