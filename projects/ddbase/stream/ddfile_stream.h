
#ifndef ddbase_stream_ddfile_stream_h_
#define ddbase_stream_ddfile_stream_h_

#include "ddbase/stream/ddistream.h"
#include "ddbase/ddnocopyable.hpp"
#include "ddbase/file/ddfile.h"
#include <memory>

namespace NSP_DD {
class ddfile_stream : public ddistream
{
    DDNO_COPY_MOVE(ddfile_stream);
    ddfile_stream() = default;
public:
    static std::unique_ptr<ddfile_stream> create_instance(const std::wstring& path);
    virtual ~ddfile_stream() = default;

public:
    s64 pos() const override;
    s64 seek(s64 offset, int origin) override;
    s64 size() const override;
    s64 resize(s64 new_size) override;
    s32 read(u8* const buff, s32 count) override;
    s32 write(const u8* const buff, s32 count) override;

protected:
    std::unique_ptr<ddfile> m_file;
    std::wstring m_path;
};

} // namespace NSP_DD
#endif // ddbase_stream_ddfile_stream_h_
