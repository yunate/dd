
#ifndef ddbase_stream_ddmemory_stream_h_
#define ddbase_stream_ddmemory_stream_h_

#include "ddbase/stream/ddistream.h"
namespace NSP_DD {

/** 内存流，最大内存：min(4G, 系统定义)
*/
class ddmemory_stream : public ddistream
{
public:
    virtual ~ddmemory_stream() = default;
    ddmemory_stream() = default;
    ddmemory_stream(s32 size);
    ddmemory_stream(const s8* const buff, s32 size);
    ddmemory_stream(ddmemory_stream&& stream) noexcept;
    ddmemory_stream(const ddistream& stream);
    ddmemory_stream(const ddmemory_stream& stream);
    ddmemory_stream& operator=(const ddistream& stream);
    ddmemory_stream& operator=(const ddmemory_stream& stream);
    ddmemory_stream& operator=(ddmemory_stream&& stream) noexcept;

    s64 pos() const override;
    s64 seek(s64 offset, int origin) override;
    s64 size() const override;
    s64 resize(s64 new_size) override;
    s32 read(u8* const buff, s32 count) override;
    s32 write(const u8* const buff, s32 count) override;

protected:
    s32 m_pos = 0;
    std::vector<u8> m_buff;
};

} // namespace NSP_DD
#endif // ddbase_stream_ddmemory_stream_h_
