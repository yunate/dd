#include "test/stdafx.h"

#include "ddbase/stream/ddfile_stream.h"
#include "ddbase/stream/ddmemory_stream.h"
#include "ddbase/ddtest_case_factory.h"

namespace NSP_DD {

DDTEST(test_stream1, ddmemory_stream)
{
    ddmemory_stream* pDogStream = new ddmemory_stream(100);
    s8 buff[128] = { 'a' };
    buff[127] = 0;
    pDogStream->write((u8*)buff, 128);
    ddmemory_stream stream1 = (*pDogStream);
    stream1.seek(1, 0);
    stream1.write((u8*)buff, 127);
    stream1.resize(200);
    stream1.resize(1025);
    stream1.resize(0x40000000);
    delete pDogStream;
}

DDTEST(test_stream, FileStream)
{
    auto file_stream = ddfile_stream::create_instance(L"filestream.txt");
    char buff[] = "hello file stream";
    file_stream->write((u8*)buff, sizeof(buff));
    file_stream->write((u8*)buff, sizeof(buff));
    s64 size = file_stream->size();

    file_stream->seek(0, SEEK_SET);
    std::vector<u8> buff1((size_t)size);
    file_stream->read(buff1.data(), (s32)size);

    ddmemory_stream memory_stream(*file_stream.get());
    memory_stream.seek(0, SEEK_SET);
    memory_stream.read(buff1.data(), (s32)size);
}
} // namespace NSP_DD
