
#include "test/stdafx.h"
#include "ddbase/ddtest_case_factory.h"

#include "ddbase/file/dddir.h"
#include "ddbase/file/ddini_file.h"
#include "ddbase/str/ddstr.h"
#include <iostream>

namespace NSP_DD {
DDTEST(test_file_utils, ini_file)
{
    auto ini_file = ddini_file::create_obj(L"E:\\aa\\1.ini", true);

    if (!ini_file) {
        return;
    }

    {
        std::vector<std::wstring> section_names;
        ini_file->get_all_section_name(section_names);
        std::vector<std::wstring> key_names;
        ini_file->get_all_key_name(L"Sec1", key_names);
        std::wstring value;
        ini_file->get_value(L"Sec1", L"key1", value);
    }

    ini_file->add_key(L"Sec1", L"key1", L"1");
    ini_file->add_key(L"Sec1", L"key2", L"1");
    ini_file->add_key(L"Sec1", L"key3", L"1");
    ini_file->add_key(L"Sec1", L"key4", L"1");

    ini_file->add_key(L"Sec2", L"key1", L"1");
    ini_file->add_key(L"Sec2", L"key2", L"1");
    ini_file->add_key(L"Sec3", L"key3", L"1");
    ini_file->add_key(L"Sec3", L"key4", L"1");

    {
        std::vector<std::wstring> section_names;
        ini_file->get_all_section_name(section_names);
        std::vector<std::wstring> key_names;
        ini_file->get_all_key_name(L"Sec1", key_names);
        std::wstring value;
        ini_file->get_value(L"Sec1", L"key1", value);
        ini_file->get_value(L"Sec1", L"key2", value);
        ini_file->get_value(L"Sec1", L"key3", value);
        ini_file->get_value(L"Sec1", L"key4", value);
        ini_file->get_value(L"Sec1", L"key5", value);
    }

    ini_file->add_key(L"Sec2", L"key1", L"1");
    ini_file->add_key(L"Sec2", L"key2", L"1");
    ini_file->add_key(L"Sec2", L"key3", L"1");
    ini_file->add_key(L"Sec2", L"key4", L"1");

    ini_file->delete_key(L"Sec1", L"key1");
    ini_file->delete_section(L"sec2");
    ini_file->add_key(L"sec1", L"key2", L"2");

    delete ini_file;
}
} // namespace NSP_DD
