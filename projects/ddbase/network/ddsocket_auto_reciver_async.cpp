#include "ddbase/stdafx.h"
#include "ddbase/network/ddnetwork_async_caller.hpp"
#include "ddbase/network/ddsocket_auto_reciver_async.h"

namespace NSP_DD {
ddsocket_auto_reciver_async::ddsocket_auto_reciver_async(std::shared_ptr<ddsocket_async> socket)
{
    DDASSERT(socket != nullptr);
    m_socket = socket;
}

ddsocket_auto_reciver_async::~ddsocket_auto_reciver_async()
{
    m_socket.reset();
}

void ddsocket_auto_reciver_async::start_recv()
{
    DDASSERT_FMTW(m_parser != nullptr, L"should call set_parser() function to set the parser.");
    DDASSERT_FMTW(shared_from_this() != nullptr, L"must use shared_ptr to manage bare point!");
    std::lock_guard<std::recursive_mutex> locker(m_socket->m_mutex);
    if (!m_buff.empty()) {
        return;
    }

    m_buff.resize(2048);
    m_buff_remain_size = 0;
    recv();
}

void ddsocket_auto_reciver_async::parse_recved_data(bool successful, s32 recved)
{
    auto weak = weak_from_this();
    DDNETWORK_ASYNC_CALL([weak, this, successful, recved]() {
        auto it = weak.lock();
        if (it == nullptr) {
            return;
        }
        if (!successful) {
            on_error();
            return;
        }

        m_buff_remain_size += (s32)(recved);
        while (true) {
            const dddata_parse_result& result = m_parser->continue_parse_from_buff(m_buff.data(), m_buff_remain_size);
            if (result.parse_state == dddata_parse_state::error) {
                on_error();
                return;
            }

            if (!on_recv(m_parser->get_context())) {
                on_error();
                return;
            }

            if (result.parsed_size > 0) {
                DDASSERT(result.parsed_size <= m_buff_remain_size);
                m_buff_remain_size -= result.parsed_size;
                if (m_buff_remain_size != 0) {
                    (void)::memmove(m_buff.data(), m_buff.data() + result.parsed_size, m_buff_remain_size);
                }
            }

            if (result.parse_state == dddata_parse_state::need_more_data ||
                m_buff_remain_size == 0) {
                recv();
                return;
            }

            if (result.parse_state == dddata_parse_state::complete) {
                m_parser->reset();
                recv();
                return;
            }
        }
    });
}

void ddsocket_auto_reciver_async::set_parser(ddidata_parser* parser)
{
    DDASSERT(parser != nullptr);
    m_parser = parser;
}

void ddsocket_auto_reciver_async::recv()
{
    auto weak = weak_from_this();
    m_socket->recv(m_buff.data() + m_buff_remain_size, (s32)(m_buff.size() - m_buff_remain_size), [weak](bool successful, u64 recved) {
        auto share = weak.lock();
        if (share != nullptr) {
            share->parse_recved_data(successful, (s32)recved);
        }
    });
}

} // namespace NSP_DD
