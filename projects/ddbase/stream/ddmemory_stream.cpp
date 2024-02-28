
#include "ddbase/stdafx.h"
#include "ddbase/stream/ddmemory_stream.h"
#include "ddbase/ddassert.h"
#include "ddbase/dderror_code.h"
#include "ddbase/ddmath.h"

namespace NSP_DD {

ddmemory_stream::ddmemory_stream(s32 size)
{
    m_buff.resize(size);
}

ddmemory_stream::ddmemory_stream(const s8* const buff, s32 size)
{
    m_buff.resize(size);
    (void)::memcpy_s(m_buff.data(), size, buff, size);
}

ddmemory_stream::ddmemory_stream(ddmemory_stream&& stream) noexcept
{
    *this = stream;
}

ddmemory_stream::ddmemory_stream(const ddistream& stream)
{
    *this = stream;
}

ddmemory_stream::ddmemory_stream(const ddmemory_stream& stream)
{
    *this = stream;
}

ddmemory_stream& ddmemory_stream::operator=(const ddistream& stream)
{
    ddistream* r = (ddistream*)(&stream);
    s64 pos = r->pos();
    (void)r->seek(0, SEEK_SET);

    s64 size = r->size();
    (void)resize(size);
    (void)r->read(m_buff.data(), (s32)size);

    (void)r->seek(pos, SEEK_SET);
    m_pos = 0;
    return *this;
}

ddmemory_stream& ddmemory_stream::operator=(const ddmemory_stream& stream)
{
    m_buff = stream.m_buff;
    m_pos = 0;
    return *this;
}

ddmemory_stream& ddmemory_stream::operator=(ddmemory_stream&& stream) noexcept
{
    m_buff = std::move(stream.m_buff);
    stream.m_pos = 0;
    return *this;
}

s64 ddmemory_stream::pos() const
{
    return m_pos;
}

s64 ddmemory_stream::seek(s64 offset, int origin)
{
    DDASSERT(offset <= MAX_S32);
    s64 pos = m_pos;
    switch (origin) {
        case SEEK_END: {
            pos = size() - offset;
            break;
        }
        case SEEK_SET: {
            pos = offset;
            break;
        }
        default: {
            // SEEK_CUR 和其他
            pos += offset;
            break;
        }
    }

    if (pos > size()) {
        pos = size();
    }

    if (pos < 0) {
        pos = 0;
    }

    m_pos = (s32)pos;
    return m_pos;
}

s64 ddmemory_stream::size() const
{
    return (s64)m_buff.size();
}

s64 ddmemory_stream::resize(s64 new_size)
{
    m_buff.resize((size_t)new_size);
    return new_size;
}

s32 ddmemory_stream::read(u8* const buff, s32 count)
{
    DDASSERT(buff != nullptr);
    DDASSERT(count > 0);

    if (m_pos == size()) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return 0;
    }

    if (MAX_S32 - count < m_pos) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return 0;
    }

    if (count + m_pos > size()) {
        count = (s32)size() - m_pos;
    }

    (void)::memcpy_s(buff, count, m_buff.data() + m_pos, count);
    m_pos += count;
    return count;
}

s32 ddmemory_stream::write(const u8* const buff, s32 count)
{
    DDASSERT(buff != nullptr);
    DDASSERT(count > 0);

    if (MAX_S32 - count < m_pos) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return 0;
    }

    if (count + m_pos > size()) {
        resize(count + m_pos);
    }

    (void)::memcpy_s(m_buff.data() + m_pos, count, buff, count);
    m_pos += count;
    return count;
}

} // namespace NSP_DD
