
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/file/ddpath.h"

#include "ddbase/ddassert.h"
#include <iostream>

namespace NSP_DD {

DDTEST(test_ddpath, name)
{
    DDASSERT(ddpath::name(L"F:\\My\\test_folder\\winrt\\MyWinrt1") == L"MyWinrt1");
    DDASSERT(ddpath::name(L"F:\\My\\test_folder\\winrt\\MyWinrt1\\") == L"");
}
DDTEST(test_ddpath, normal)
{
    {
        std::wstring path = L"\\";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"");
    }
    {
        std::wstring path = L"\\\\";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"");
    }
    {
        std::wstring path = L"\\a";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"\\a");
    }
    {
        std::wstring path = L"\\\\a";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"\\a");
    }
    {
        std::wstring path = L"\\\\a\\";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"\\a");
    }
    {
        std::wstring path = L"";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"");
    }
    {
        std::wstring path = L"F:\\My\\test_folder\\winrt\\MyWinrt1";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"F:\\My\\test_folder\\winrt\\MyWinrt1");
    }
    {
        std::wstring path = L"F:\\\\My\\\\test_folder\\\\winrt\\\\MyWinrt1\\";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"F:\\My\\test_folder\\winrt\\MyWinrt1");
    }

    {
        std::wstring path = L"F://My/test_folder/winrt/MyWinrt1";
        ddpath::normal(path, L'\\');
        DDASSERT(path == L"F:\\My\\test_folder\\winrt\\MyWinrt1");
    }
}
DDTEST(test_ddpath, join)
{
    std::wstring expect = L"F:\\My\\test_folder\\winrt\\MyWinrt1";
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\";
        std::wstring right = L"\\MyWinrt1";
        DDASSERT(ddpath::join(left, right) == expect);
    }
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt";
        std::wstring right = L"\\MyWinrt1";
        DDASSERT(ddpath::join(left, right) == expect);
    }
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\";
        std::wstring right = L"MyWinrt1";
        DDASSERT(ddpath::join(left, right) == expect);
    }
}

DDTEST(test_ddpath, suffix)
{
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\MyWinrt1.txt";
        DDASSERT(ddpath::suffix(left) == L"txt");
    }
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\MyWinrt1";
        DDASSERT(ddpath::suffix(left) == L"");
    }
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\MyWinrt1\\";
        DDASSERT(ddpath::suffix(left) == L"");
    }
    {
        std::wstring left = L"";
        DDASSERT(ddpath::suffix(left) == L"");
    }
}

DDTEST(test_ddpath, parent)
{
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\MyWinrt1.txt";
        DDASSERT(ddpath::parent(left) == L"F:\\My\\test_folder\\winrt");
    }
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\MyWinrt1";
        DDASSERT(ddpath::parent(left) == L"F:\\My\\test_folder\\winrt");
    }
    {
        std::wstring left = L"F:\\My\\test_folder\\winrt\\MyWinrt1\\";
        DDASSERT(ddpath::parent(left) == L"F:\\My\\test_folder\\winrt\\MyWinrt1");
    }
    {
        std::wstring left = L"";
        DDASSERT(ddpath::parent(left) == L"");
    }
    {
        std::wstring left = L"MyWinrt1";
        DDASSERT(ddpath::parent(left) == L"");
    }
}

DDTEST(test_ddpath_expand, expand)
{
    {
        std::wstring src = L"E:/a/b/../d/e/f";
        std::wstring expect = L"E:/a/d/e/f";
        DDASSERT(ddpath::expand1(src, L'/') == expect);
    }
  
    {
        std::wstring src = L"E:/a/b/../../d/e/f";
        std::wstring expect = L"E:/d/e/f";
        DDASSERT(ddpath::expand1(src, L'/') == expect);
    }

    {
        std::wstring src = L"E:/a/b/./../../../d/e/f";
        std::wstring expect = L"d/e/f";
        DDASSERT(ddpath::expand1(src, L'/') == expect);
    }

    {
        std::wstring src = L"E:/a/b/./../../../../d/e/f";
        std::wstring expect = L"../d/e/f";
        DDASSERT(ddpath::expand1(src, L'/') == expect);
    }
}

DDTEST(test_ddpath_relative, relative_path)
{
    {
        std::wstring base = L"E:/a/b/c/d/e/f";
        std::wstring src = L"E:/a/b/c/d/e/f";
        std::wstring expect = L"";
        DDASSERT(ddpath::relative_path(base, src, L'/') == expect);
    }

    {
        std::wstring base = L"E:/a/b/c/d/e/f";
        std::wstring src = L"E:/a/b/c/d/e/";
        std::wstring expect = L"..";
        DDASSERT(ddpath::relative_path(base, src, L'/') == expect);
    }

    {
        std::wstring base = L"E:/a/b/c/d/e/f";
        std::wstring src = L"E:/a/b/c/d/";
        std::wstring expect = L"../..";
        DDASSERT(ddpath::relative_path(base, src, L'/') == expect);
    }

    {
        std::wstring base = L"E:/a/b/c/d/e";
        std::wstring src = L"E:/a/b/c/d/e/f";
        std::wstring expect = L"f";
        DDASSERT(ddpath::relative_path(base, src, L'/') == expect);
    }

    {
        std::wstring base = L"E:/a/b/c/d";
        std::wstring src = L"E:/a/b/c/d/e/f";
        std::wstring expect = L"e/f";
        DDASSERT(ddpath::relative_path(base, src, L'/') == expect);
    }

    {
        std::wstring base = L"E:/a/b/z/d/e/f";
        std::wstring src = L"E:/a/b/c/d/e/f";
        std::wstring expect = L"../../../../c/d/e/f";
        DDASSERT(ddpath::relative_path(base, src, L'/') == expect);
    }

    {
        std::wstring base = L"E:/a/b/";
        std::wstring src = L"C:/a/b/c/d/e/f";
        std::wstring expect = L"../../../C:/a/b/c/d/e/f";
        DDASSERT(ddpath::relative_path(base, src, L'/') == expect);
    }
}
} // namespace NSP_DD
