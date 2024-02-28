
#include "test/stdafx.h"
#include "ddbase/file/ddfile.h"
#include "ddbase/file/dddir.h"
#include "ddbase/ddassert.h"
#include "ddbase/ddtest_case_factory.h"
#include "ddbase/windows/ddmoudle_utils.h"
#include <iterator>

namespace NSP_DD {
DDTEST(test_ddfile_filesize, ddfile)
{
    std::wstring path = ddmoudle_utils::get_moudle_pathW();
    s64 file_size = ddfile::file_size(path);
    DDASSERT(file_size > 0);
}

DDTEST(test_ddfile_get_file_type, main1)
{
    ddfile_type type = ddfile::get_file_type(LR"__(E:\ddworkspace\test.txt)__");
    DDASSERT(type != ddfile_type::unknown);
}

DDTEST(test_ddfile, ddfile)
{
    {
        {
            std::shared_ptr<ddfile> file(ddfile::create_usc2_file(L".\\test_usc2.tmp"));
            DDASSERT(file != nullptr);
        }

        {
            std::shared_ptr<ddfile> file(ddfile::create_utf8bom_file(L".\\test_usc2.tmp"));
            DDASSERT(file == nullptr);
        }

        {
            std::shared_ptr<ddfile> file(ddfile::create_usc2_file(L".\\test_usc2.tmp"));
            DDASSERT(file != nullptr);
            (void)file->resize(2);
            std::wstring w = L"你好，文件\r\n";
            for (int i = 0; i < 1000; ++i) {
                file->write((u8*)w.c_str(), (u32)w.length() * 2);
            }

            (void)file->seek(2);
            std::wstring r;
            for (int i = 0; i < 1000; ++i) {
                file->read_linew(r);
                DDASSERT(r == w);
                r.clear();
            }
        }

        {
            std::wstring w = L"你好，文件\r\n";
            {
                std::shared_ptr<ddfile> file(ddfile::create_usc2_file(L".\\test_usc2.tmp"));
                DDASSERT(file != nullptr);
                (void)file->seek(-1);
                for (int i = 0; i < 1000; ++i) {
                    file->write((u8*)w.c_str(), (u32)w.length() * 2);
                }
            }

            
            {
                std::shared_ptr<ddfile> file(ddfile::create_usc2_file(L".\\test_usc2.tmp"));
                DDASSERT(file != nullptr);
                std::wstring r;
                for (int i = 0; i < 2000; ++i) {
                    file->read_linew(r);
                    DDASSERT(r == w);
                    r.clear();
                }
            }
        }
    }

    {
        std::shared_ptr<ddfile> file(ddfile::create_usc2_file(L".\\test_usc2.tmp"));
        DDASSERT(file != nullptr);
        (void)file->seek(-1);
        s64 size = file->file_size();
        s64 offset = file->current_offset();
        DDASSERT(size == offset);
        (void)file->seek(0);
        offset = file->current_offset();
        DDASSERT(0 == offset);
        (void)file->seek(-1);
        offset = file->current_offset();
        DDASSERT(size == offset);
        (void)file->resize(0);
        size = file->file_size();
        DDASSERT(size == 0);
    }

    dddir::delete_path(L".\\test_usc2.tmp");
}

DDTEST(test_ddfile1, ddfile)
{
    {
        {
            std::shared_ptr<ddfile> file(ddfile::create_utf8_file(L".\\test_usc2.tmp"));
            DDASSERT(file != nullptr);
            file->resize(0);
        }

        //{
        //    std::shared_ptr<ddfile> file(ddfile::create_utf8bom_file(L".\\test_usc2.tmp"));
        //    DDASSERT(file != nullptr);
        //}

        {
            std::shared_ptr<ddfile> file(ddfile::create_utf8_file(L".\\test_usc2.tmp"));
            DDASSERT(file != nullptr);
            (void)file->resize(0);
            std::string w = "你好，文件\r\n";
            for (int i = 0; i < 1000; ++i) {
                file->write((u8*)w.c_str(), (u32)w.length());
            }

            (void)file->seek(0);
            std::string r;
            for (int i = 0; i < 1000; ++i) {
                file->read_linea(r);
                DDASSERT(r == w);
                r.clear();
            }
        }

        {
            std::string w = "你好，文件\r\n";
            {
                std::shared_ptr<ddfile> file(ddfile::create_utf8_file(L".\\test_usc2.tmp"));
                DDASSERT(file != nullptr);
                (void)file->seek(-1);
                for (int i = 0; i < 1000; ++i) {
                    file->write((u8*)w.c_str(), (u32)w.length());
                }
            }


            {
                std::shared_ptr<ddfile> file(ddfile::create_utf8_file(L".\\test_usc2.tmp"));
                DDASSERT(file != nullptr);
                std::string r;
                for (int i = 0; i < 2000; ++i) {
                    file->read_linea(r);
                    DDASSERT(r == w);
                    r.clear();
                }
            }
        }
    }

    {
        std::shared_ptr<ddfile> file(ddfile::create_utf8_file(L".\\test_usc2.tmp"));
        DDASSERT(file != nullptr);
        (void)file->seek(-1);
        s64 size = file->file_size();
        s64 offset = file->current_offset();
        DDASSERT(size == offset);
        (void)file->seek(0);
        offset = file->current_offset();
        DDASSERT(0 == offset);
        (void)file->seek(-1);
        offset = file->current_offset();
        DDASSERT(size == offset);
        (void)file->resize(0);
        size = file->file_size();
        DDASSERT(size == 0);
    }

    dddir::delete_path(L".\\test_usc2.tmp");
}

DDTEST(test_ddfile2, ddfile)
{
    std::string finder_line = "abc-def--\n";
    std::string replacer_line = "ABC--DEFF\n";
    ddbuff finder;
    ddbuff replacer;
    std::copy(finder_line.begin(), finder_line.end(), std::back_inserter(finder));
    std::copy(replacer_line.begin(), replacer_line.end(), std::back_inserter(replacer));
    std::vector<ddbuff> finders{ finder };
    std::vector<ddbuff> replacers{ replacer };
    int lines = 1536000;

    std::wstring src_path = L"F:\\My\\test_folder\\test_file_replacer.txt";
    std::wstring out_path = L"F:\\My\\test_folder\\test_file_replacer1.txt";
    if (dddir::is_path_exist(src_path)) {
        dddir::delete_path(src_path);
    }
    if (dddir::is_path_exist(out_path)) {
        dddir::delete_path(out_path);
    }

    std::shared_ptr<ddfile> src(ddfile::create_utf8_file(src_path));
    for (int i = 0; i < lines; ++i) {
        (void)src->write((u8*)finder_line.data(), (s32)finder_line.size());
    }
    src->seek(0);

    std::shared_ptr<ddfile> out(ddfile::create_utf8_file(out_path));
    ddfile::file_replace(src.get(), out.get(), finders, replacers);
    out->seek(0);
    std::string readed_line;
    int j = 0;
    while (out->read_linea(readed_line)) {
        DDASSERT(replacer_line == readed_line);
        ++j;
    }
    DDASSERT(j == lines);
}

} // namespace NSP_DD
