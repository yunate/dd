#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/file/ddpath.h"
#include "ddbase/file/dddir.h"

#include "ddbase/ddassert.h"
#include "ddbase/ddio.h"
#include "ddbase/ddtime.h"
#include <iostream>

namespace NSP_DD {
DDTEST(test_dddir, copy_path_ex)
{
    std::vector<std::wstring> include_file = { L"lib", L"dll", L"exe", L"pdb", L"h", L"hpp" };
    const std::wstring src_full_path = LR"__(F:\My\ddwork_space\dd)__";
    const std::wstring dst_full_path = LR"__(F:\My\ddwork_space\ddexport)__";
    std::wstring current_copy_path;
    if (dddir::copy_path_ex(src_full_path, dst_full_path, [&include_file, &current_copy_path](const std::wstring& full_path) {
        if (full_path.find(L"\\tmp\\") != std::wstring::npos) {
            return true;
        }
        if (full_path.find(L"\\__DD_DEMO__\\") != std::wstring::npos) {
            return true;
        }
        std::wstring suffix = ddpath::suffix(full_path);
        for (const auto& it : include_file) {
            if (suffix == it) {
                current_copy_path = full_path;
                return false;
            }
        }
        return true;
    })) {
        ddcout(ddconsole_color::green) << L"successful!\n";
    } else {
        ddcout(ddconsole_color::red) << L"copy file failure: " << current_copy_path << L"\n";
        ddcout(ddconsole_color::red) << L"error: " << last_error_msgw(::GetLastError()) << L"\n";
    }
}

DDTEST(test_dddir, create_dir_ex)
{
    dddir::create_dir_ex(L"E:\\aa\\bb\\cc");
    dddir::create_dir_ex(L"E:\\aa\\bb1\\cc1\\");
    dddir::create_dir_ex(L"E:/aa/bb2\\cc2\\");
    dddir::create_dir_ex(L"E:/aa/bb2/cc2/");

    std::vector<std::wstring> expect
    {
        L"E:\\aa\\bb",
        L"E:\\aa\\bb1",
        L"E:\\aa\\bb2",
        L"E:\\aa\\bb\\cc",
        L"E:\\aa\\bb1\\cc1",
        L"E:\\aa\\bb2\\cc2"
    };

    {
        std::vector<std::wstring> out;
        dddir::enum_dir(L"E:\\aa", [&out](const::std::wstring& sub_path, const std::wstring& name, bool is_dir) {
            if (is_dir) {
                out.push_back(ddpath::join({ L"E:\\aa", sub_path, name }));
            }
            return false;
        });
        DDASSERT(expect == out);
    }

    {
        std::vector<std::wstring> out;
        dddir::enum_dir(L"E:\\aa", [&out](const::std::wstring& path, bool is_dir) {
            if (is_dir) {
                out.push_back(path);
            }
            return false;
        });
        DDASSERT(expect == out);
    }
}

DDTEST(test_dddir, enum_dir)
{
    std::wstring dir_path = L"E:\\VM";
    std::vector<std::wstring> pathAllVec;
    dddir::enum_dir(dir_path, pathAllVec, nullptr);

    std::vector<std::wstring> pathAllDir;
    dddir::enum_dir(dir_path, pathAllDir, [](const std::wstring&, bool is_dir) {
        if (is_dir) {
            return true;
        }
        return false;
    });

    std::vector<std::wstring> pathAllFile;
    dddir::enum_dir(dir_path, pathAllFile, [](const std::wstring&, bool is_dir) {
        if (is_dir) {
            return false;
        }
        return true;
    });

    std::vector<std::wstring> pathAllTxt;
    dddir::enum_dir(dir_path, pathAllTxt, [](const std::wstring& path, bool is_dir) {
        if (is_dir) {
            return false;
        }
        if (path.length() >= 4 &&
            path[path.length() - 4] == L'.' &&
            path[path.length() - 3] == L't' &&
            path[path.length() - 2] == L'x' &&
            path[path.length() - 1] == L't') {
            return true;
        }
        return false;
    });
}


DDTEST(test_dddir1, copy)
{
    DDASSERT(dddir::copy_path(L"F:\\My\\test_folder\\test_rename_src1", L"D:\\My\\test_folder\\test_rename_src1"));
}

DDTEST(test_dddir1, rename)
{
    DDASSERT(dddir::rename_path(L"F:\\My\\test_folder\\test_rename_src3", L"F:\\My\\test_folder\\test_rename_src2"));
}

DDTEST(test_dddir2, delete_path)
{
    {
        DDASSERT(dddir::create_dir_ex(L"F:\\My\\test_folder\\test_delete_path_dir"));
        DDASSERT(dddir::create_file(L"F:\\My\\test_folder\\test_delete_path_dir\\test_delete_path_file"));
        DDASSERT(dddir::delete_path(L"F:\\My\\test_folder\\test_delete_path_dir\\test_delete_path_file"));
        DDASSERT(dddir::delete_path(L"F:\\My\\test_folder\\test_delete_path_dir"));
    }

    {
        dddir::create_dir_ex(L"F:\\My\\test_folder\\aa\\bb\\cc");
        dddir::create_dir_ex(L"F:\\My\\test_folder\\aa\\bb1\\cc1\\");
        dddir::create_dir_ex(L"F:\\My\\test_folder/aa/bb2\\cc2\\");
        dddir::create_dir_ex(L"F:\\My\\test_folder/aa/bb2/cc2/");
        dddir::create_file(L"F:\\My\\test_folder/aa/bb2/cc2/ttt.txt");
        std::vector<dddir::enum_dir_info> result;
        dddir::enum_dir_first_level(L"F:\\My\\test_folder\\aa", result);
        DDASSERT(dddir::delete_path(L"F:\\My\\test_folder\\aa"));
    }
}


DDTEST(test_dddir4, 1)
{
    u64 file_count = 0;
    u64 dir_count = 0;
    ddtimer timer;
    std::wstring root_dir = LR"(C:\)";
    std::queue<std::wstring> path_queue;
    path_queue.push(root_dir);
    while (!path_queue.empty()) {
        const std::wstring& path = path_queue.front();
        std::vector<dddir::enum_dir_info> result;
        dddir::enum_dir_first_level(path, result);
        for (const auto& it : result) {
            if (it.is_dir) {
                path_queue.push(ddpath::join({ path, it.name }));
                ++dir_count;
            } else {
                ++file_count;
            }
        }
        path_queue.pop();
    }

    ddcout(gray) << ddstr::format("dir %d, file %d\n", dir_count, file_count);
    ddcout(gray) << ddstr::format("emum [%s] dir, cost [%d]ms\n", root_dir.c_str(), timer.get_time_pass() / 1000 / 1000);
}
} // namespace NSP_DD
