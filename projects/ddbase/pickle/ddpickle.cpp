#include "ddbase/stdafx.h"
#include "ddbase/pickle/ddpickle.h"
namespace NSP_DD {

#pragma pack(push, 1)
struct ddpickle_header
{
    u32 pickle_size = 0;
};
#pragma pack(pop)

u32 ddpickle::get_next_pickle_size(const u8* buff, u32 buff_size)
{
    if (buff == nullptr || buff_size == 0) {
        return 0;
    }

    ddpickle_header* h = (ddpickle_header*)(buff);
    if (h->pickle_size > buff_size) {
        return 0;
    }
    return h->pickle_size;
}

u32 ddpickle::get_max_size()
{
    return 10 * 1024 * 1024;
}

ddpickle::ddpickle()
{
    m_buff.reserve(64);
    resize(sizeof(ddpickle_header));
    reset_read_seek();
}

ddpickle::ddpickle(const u8* buff, u32 buff_size)
{
    u32 pickle_size = get_next_pickle_size(buff, buff_size);
    if (pickle_size == 0) {
        m_buff.reserve(64);
        resize(sizeof(ddpickle_header));
        return;
    }

    resize(pickle_size);
    (void)::memcpy_s(m_buff.data(), pickle_size, buff, pickle_size);
    reset_read_seek();
}

bool ddpickle::write_buff(u8* buff, u32 buff_size)
{
    if (buff == nullptr || buff_size == 0) {
        return false;
    }
    u32 current_size = (u32)m_buff.size();
    resize((u32)m_buff.size() + buff_size);
    (void)::memcpy_s(m_buff.data() + current_size, buff_size, buff, buff_size);
    return true;
}

const ddbuff& ddpickle::get_buff()
{
    return m_buff;
}

void ddpickle::reset_read_seek()
{
    m_read_seek = sizeof(ddpickle_header);
}

bool ddpickle::read_buff(u8* buff, u32 buff_size)
{
    if (buff == nullptr || buff_size == 0) {
        return false;
    }

    if (m_read_seek + buff_size > m_buff.size()) {
        return false;
    }

    (void)::memcpy_s(buff, buff_size, m_buff.data() + m_read_seek, buff_size);
    m_read_seek += buff_size;
    return true;
}

void ddpickle::resize(u32 size)
{
    if (size < sizeof(ddpickle_header)) {
        return;
    }

    m_buff.resize(size);
    ddpickle_header* h = (ddpickle_header*)(m_buff.data());
    (*h).pickle_size = size;
}
} // namespace NSP_DD

