#include "ddbase/stdafx.h"
#include "ddbase/stream/ddstream_view.h"
namespace NSP_DD {
class ddstream_views : public ddistream_view
{
public:
    ddstream_views(std::vector<std::unique_ptr<ddistream_view>>&& stream_views)
    {
        m_items = std::move(stream_views);
        for (auto& it : m_items) {
            m_all_size += it->size();
        }
    }

    virtual ~ddstream_views() { };

    s64 pos() const override
    {
        return m_pos;
    }

    s64 size() const override
    {
        return m_all_size;
    }

    s32 read(u8* const buff, s32 count) override
    {
        DDASSERT(buff != nullptr);
        s64 readed = 0;
        u8* buff_pos = buff;
        s32 need_reade_count = count;

        while (true) {
            if (m_index >= (s32)m_items.size()) {
                m_pos += readed;
                return (s32)readed;
            }

            ddistream_view& item = *m_items[m_index];
            s64 item_remain_size = item.size() - item.pos();
            if (need_reade_count <= item_remain_size) {
                readed += item.read(buff_pos, need_reade_count);
                m_pos += readed;
                return (s32)readed;
            }

            readed += item.read(buff_pos, (s32)item_remain_size);
            buff_pos += item_remain_size;
            need_reade_count -= (s32)item_remain_size;
            ++m_index;
        }
    }

    s64 seek(s64 offset, int origin) override
    {
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

        m_pos = pos;
        m_index = (s32)m_items.size();
        s64 size = 0;
        for (size_t i = 0; i < m_items.size(); ++i) {
            size += (s64)m_items[i]->size();
            if (m_pos < size) {
                m_index = (s32)i;
                break;
            }
        }
        return m_pos;
    }

private:
    std::vector<std::unique_ptr<ddistream_view>> m_items;
    s32 m_index = 0;
    s64 m_all_size = 0;
    s64 m_pos = 0;
};

class ddstream_buff_view : public ddistream_view
{
public:
    ddstream_buff_view(u8* buff, s32 buff_size) :
        m_buff(buff), m_buff_size(buff_size)
    {
    }
    virtual ~ddstream_buff_view() { };

    s64 pos() const override
    {
        return m_pos;
    }

    s64 size() const override
    {
        return m_buff_size;
    }

    s32 read(u8* const buff, s32 count) override
    {
        if (m_buff == nullptr) {
            return 0;
        }
        s32 remain_size = (s32)(m_buff_size - m_pos);
        if (count > remain_size) {
            (void*)::memcpy(buff, m_buff + m_pos, remain_size);
            m_pos += remain_size;
            return remain_size;
        } else {
            (void*)::memcpy(buff, m_buff + m_pos, count);
            m_pos += count;
            return count;
        }
    }

    s64 seek(s64 offset, int origin) override
    {
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

private:
    u8* m_buff = nullptr;
    s32 m_buff_size = 0;
    s32 m_pos = 0;
};

class ddstream_istream_view : public ddistream_view
{
public:
    ddstream_istream_view(ddistream* stream) : m_stream(stream) { }
    virtual ~ddstream_istream_view() { }

    s64 pos() const override
    {
        if (m_stream == nullptr) {
            return 0;
        }
        return m_stream->pos();
    }

    s64 size() const override
    {
        if (m_stream == nullptr) {
            return 0;
        }
        return m_stream->size();
    }

    s32 read(u8* const buff, s32 count) override
    {
        if (m_stream == nullptr) {
            return 0;
        }
        return m_stream->read(buff, count);
    }

    s64 seek(s64 offset, int origin) override
    {
        return m_stream->seek(offset, origin);
    }

private:
    ddistream* m_stream = nullptr;
};

class ddstream_tls_encode_view : public ddistream_view
{
public:
    ddstream_tls_encode_view(ddistream_view* stream, ddtls* tls) : m_stream(stream), m_tls(tls){ }
    virtual ~ddstream_tls_encode_view() { }

    s64 pos() const override
    {
        if (m_stream == nullptr) {
            return 0;
        }
        return m_stream->pos();
    }

    s64 size() const override
    {
        if (m_stream == nullptr) {
            return 0;
        }
        return m_stream->size();
    }

    s32 read(u8* const buff, s32 count) override
    {
        if (m_stream == nullptr) {
            return 0;
        }

        s64 raw_pos = pos();

        s32 size = m_tls->get_raw_size(count);
        if (size > count) {
            size = count;
        }

        std::vector<u8> read_buff(size);
        s32 readed = m_stream->read(read_buff.data(), (s32)read_buff.size());

        if (!m_tls->encode(read_buff.data(), readed, buff, size)) {
            m_stream->seek(raw_pos, SEEK_SET);
            return 0;
        }

        if (readed > 0) {
            m_stream->seek((s64)readed * -1, SEEK_CUR);
        }

        return size;
    }

    s64 seek(s64 offset, int origin) override
    {
        return m_stream->seek(offset, origin);
    }

private:
    ddistream_view* m_stream = nullptr;
    ddtls* m_tls = nullptr;
};
} // namespace NSP_DD

namespace NSP_DD {
std::unique_ptr<ddistream_view> ddstream_view::from(void* buff, s32 buff_size)
{
    return std::make_unique<ddstream_buff_view>((u8*)buff, buff_size);
}

std::unique_ptr<ddistream_view> ddstream_view::from(ddistream* stream)
{
    return std::make_unique<ddstream_istream_view>(stream);
}

std::unique_ptr<ddistream_view> ddstream_view::from(ddistream_view* stream_view, ddtls* tls)
{
    DDASSERT(stream_view != nullptr);
    DDASSERT(tls != nullptr);
    return std::make_unique<ddstream_tls_encode_view>(stream_view, tls);
}

std::unique_ptr<ddistream_view> ddstream_view::from(std::vector<std::unique_ptr<ddistream_view>>&& stream_views)
{
    return std::make_unique<ddstream_views>(std::move(stream_views));
}
} // namespace NSP_DD
