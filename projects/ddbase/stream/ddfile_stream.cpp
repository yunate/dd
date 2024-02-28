#include "ddbase/stdafx.h"
#include "ddbase/stream/ddfile_stream.h"
#include "ddbase/ddassert.h"
#include "ddbase/dderror_code.h"
#include "ddbase/ddmath.h"
#include <tchar.h>

namespace NSP_DD {
std::unique_ptr<ddfile_stream> ddfile_stream::create_instance(const std::wstring& path)
{
    std::unique_ptr<ddfile_stream> inst(new(std::nothrow) ddfile_stream());
    if (inst == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }
    inst->m_file.reset(ddfile::create_or_open(path));
    if (inst->m_file == nullptr) {
        return nullptr;
    }
    inst->m_path = path;
    return inst;
}

s64 ddfile_stream::pos() const
{
    DDASSERT(m_file != nullptr);
    return m_file->current_offset();
}

s64 ddfile_stream::seek(s64 offset, int origin)
{
    DDASSERT(offset <= MAX_S32);
    DDASSERT(m_file != nullptr);
    s64 current_pos = this->pos();
    s64 size = this->size();
    switch (origin) {
        case SEEK_END: {
            current_pos = size - offset;
            break;
        }
        case SEEK_SET: {
            current_pos = offset;
            break;
        }
        default: {
            // SEEK_CUR 和其他
            current_pos += offset;
            break;
        }
    }

    if (current_pos > size) {
        current_pos = size;
    }

    if (current_pos < 0) {
        current_pos = 0;
    }

    if (!m_file->seek(current_pos)) {
        return this->pos();
    }
    return current_pos;
}

s64 ddfile_stream::size() const
{
    DDASSERT(m_file != nullptr);
    return m_file->file_size();
}

s64 ddfile_stream::resize(s64 new_size)
{
    DDASSERT(m_file != nullptr);
    return m_file->resize(new_size);
}

s32 ddfile_stream::read(u8* const buff, s32 count)
{
    DDASSERT(m_file != nullptr);
    DDASSERT(buff != nullptr);
    DDASSERT(count > 0);
    return m_file->read(buff, count);
}

s32 ddfile_stream::write(const u8* const buff, s32 count)
{
    DDASSERT(m_file != nullptr);
    DDASSERT(buff != nullptr);
    DDASSERT(count > 0);
    return m_file->write(buff, count);
}
} // namespace NSP_DD
