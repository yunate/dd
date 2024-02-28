#include "ddbase/stdafx.h"
#include "ddbase/network/ddtls_socket_async.h"
#include "ddbase/network/ddnetwork_async_caller.hpp"
#include "ddbase/stream/ddmemory_stream.h"
#include "ddbase/ddtimer.h"
namespace NSP_DD {

#define DDISTREAM_NO_IMPL { DDASSERT_FMTW(false, L"this function is not implemented."); return 0; }
class ddtls_stream_wrapper : public ddistream
{
    DDNO_COPY_MOVE(ddtls_stream_wrapper);
public:
    // 使用者保证stream的生命周期, stream中永远存储的是明文
    // 使用者保证tls的生命周期
    ddtls_stream_wrapper(ddistream* stream, ddtls* tls) :
        m_stream(stream), m_tls(tls)
    {

    }
    virtual ~ddtls_stream_wrapper() { };

    virtual s64 pos() const override
    {
        return m_stream->pos();
    }

    virtual s64 size() const override
    {
        return m_stream->size();
    }

    virtual s32 read(u8* const buff, s32 count) override
    {
        if (count <= m_tls->get_tls_encode_extra_size()) {
            // buff 太小
            DDASSERT(false);
            return 0;
        }

        s32 size = m_tls->get_tls_encode_max_size() + m_tls->get_tls_encode_extra_size();
        if (size > count) {
            size = count;
        }

        std::vector<u8> read_buff(size - m_tls->get_tls_encode_extra_size());
        s32 readed = m_stream->read(read_buff.data(), (s32)read_buff.size());
        if (!m_tls->encode(read_buff.data(), readed, buff, size)) {
            return 0;
        }

        if (readed > 0) {
            m_stream->seek(readed * -1, SEEK_CUR);
        }
        return size;
    }

    virtual s64 seek(s64, int) override DDISTREAM_NO_IMPL;
    virtual s64 resize(s64) override DDISTREAM_NO_IMPL;
    virtual s32 write(const u8* const, s32) override DDISTREAM_NO_IMPL;

private:
    ddistream* m_stream = nullptr;
    ddtls* m_tls = nullptr;
};

ddtls_socket_async::ddtls_socket_async(const std::shared_ptr<ddtls>& tls, const std::shared_ptr<ddsocket_async>& socket)
    : m_tls(tls), m_socket(socket)
{
    DDASSERT(m_tls != nullptr);
    DDASSERT(m_socket != nullptr);
}

std::shared_ptr<ddtls_socket_async> ddtls_socket_async::create_instance(const std::shared_ptr<ddtls>& tls, const std::shared_ptr<ddsocket_async>& socket)
{
    std::shared_ptr<ddtls_socket_async> tls_socket(new(std::nothrow) ddtls_socket_async(tls, socket));
    if (tls_socket == nullptr) {
        return nullptr;
    }

    auto* iocp = socket->get_iocp();
    if (iocp == nullptr) {
        return nullptr;
    }

    if (!iocp->watch(tls_socket)) {
        return nullptr;
    }

    return tls_socket;
}

ddtls_socket_async::~ddtls_socket_async()
{
    m_socket->get_iocp()->unwatch(m_socket.get());
}

void ddtls_socket_async::send(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout)
{
    std::shared_ptr<ddmemory_stream> mem_stream(new ddmemory_stream((s8*)buff, buff_size));
    send_stream(mem_stream.get(), [mem_stream, callback](bool successful, bool all_sended, u64 sended) {
        if (successful && all_sended) {
            callback(true, (s32)sended);
        }
        return true;
    }, timeout);
}

void ddtls_socket_async::send_stream(ddistream* stream, const ddsocket_async_callback1& callback, u64 timeout)
{
    std::shared_ptr<ddtls_stream_wrapper> wrappered_stream(new ddtls_stream_wrapper(stream, m_tls.get()));
    m_socket->send_stream(wrappered_stream.get(), [wrappered_stream, callback](bool successful, bool all_sended, u64 sended){
        return callback(successful, all_sended, sended);
    }, timeout);
}

void ddtls_socket_async::recv(void* buff, s32 buff_size, const ddsocket_async_callback& callback, u64 timeout)
{
    if (m_decoded_size > 0) {
        if (m_decoded_size <= buff_size) {
            ::memmove(buff, m_buff.data() + m_decoded_data_begin_pos, m_decoded_size);
            s32 return_size = m_decoded_size;
            m_decoded_size = 0;
            if (m_encoded_size > 0) {
                ::memmove(m_buff.data(), m_buff.data() + m_encoded_data_begin_pos, m_encoded_size);
                m_encoded_data_begin_pos = 0;
            } else {
                m_buff.resize(0);
                m_buff.shrink_to_fit();
            }
            m_decoded_data_begin_pos = 0;
            callback(true, return_size);
        } else {
            ::memmove(buff, m_buff.data() + m_decoded_data_begin_pos, buff_size);
            m_decoded_size -= (s32)buff_size;
            m_decoded_data_begin_pos += (s32)buff_size;
            callback(true, buff_size);
        }

        return;
    }

    if (m_encoded_size == 0) {
        m_buff.resize((s64)m_tls->get_tls_encode_max_size() + m_tls->get_tls_encode_extra_size());
    }

    u64 experid_timestamp = ddtime::get_experid_timestamp(timeout);
    m_socket->recv(m_buff.data() + m_encoded_size, (s32)m_buff.size() - m_encoded_size, [this, callback, buff_size, buff, experid_timestamp](bool successful, s32 recved) {
        auto weak = get_iocp()->get_weaker(this);
        DDNETWORK_ASYNC_CALL([weak, this, successful, callback, recved, buff, buff_size, experid_timestamp]() {
            auto it = weak.lock();
            if (it == nullptr) {
                return;
            }

            if (!successful) {
                callback(false, 0);
                return;
            }

            m_encoded_size += recved;
            s32 raw_size = m_encoded_size;
            if (!m_tls->decode(m_buff.data(), m_encoded_size, m_decoded_size)) {
                callback(false, 0);
                return;
            }

            if (m_encoded_size == (s32)m_buff.size()) {
                callback(false, 0);
                return;
            }

            m_decoded_data_begin_pos = 0;
            m_encoded_data_begin_pos = raw_size - m_encoded_size;
            recv(buff, buff_size, callback, ddtime::get_timeout(experid_timestamp));
        });
    }, timeout);
}

struct ddtls_handshake_async_ctx
{
    ddtls_handshake_async_ctx(s32 size, const std::function<void(bool)>& callback)
    {
        out_buff.resize(size);
        in_buff.resize(size);
        this->callback = callback;
    }

    std::vector<u8> out_buff;
    std::vector<u8> in_buff;
    s32 in_buff_size = 0;
    s32 out_buff_size = 0;
    std::function<void(bool)> callback;
};


void ddtls_socket_async::hand_shake(const std::function<void(bool)>& callback)
{
    DDASSERT_FMTW(m_handshake_ctx == nullptr, L"pre hand_shake is still running.");
    m_handshake_ctx = new(std::nothrow) ddtls_handshake_async_ctx(m_tls->get_hand_shake_buff_size(), callback);
    if (m_handshake_ctx == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        callback(false);
        return;
    }
    continue_hand_shake_inner();
}

bool ddtls_socket_async::virtual_dispatch()
{
    return true;
}

void ddtls_socket_async::continue_hand_shake_inner()
{
    auto weak = get_iocp()->get_weaker(this);
    DDNETWORK_ASYNC_CALL([weak, this]() {
        auto it = weak.lock();
        if (it == nullptr) {
            return;
        }
        ddtls_handshake_async_ctx* ctx = (ddtls_handshake_async_ctx*)(m_handshake_ctx);
        ddtls_handshake_ctx* tls_ctx = m_tls->get_handshanke_ctx();
        if (tls_ctx->need_send) {
            tls_ctx->need_send = false;
            m_socket->send(ctx->out_buff.data(), ctx->out_buff_size, [ctx, this](bool successful, s32 sended) {
                if (!successful || sended != ctx->out_buff_size) {
                    end_hand_shake(false);
                    return;
                }

                continue_hand_shake_inner();
            });
            return;
        }

        if (tls_ctx->need_recv) {
            tls_ctx->need_recv = false;
            m_socket->recv(ctx->in_buff.data() + ctx->in_buff_size, (s32)ctx->in_buff.size() - ctx->in_buff_size, [ctx, this](bool successful, s32 recv) {
                if (!successful) {
                    end_hand_shake(false);
                    return;
                }

                ctx->in_buff_size += recv;
                continue_hand_shake_inner();
            });
            return;
        }

        if (tls_ctx->complete) {
            tls_ctx->complete = false;
            end_hand_shake(true);
            return;
        }

        ctx->out_buff_size = (s32)ctx->out_buff.size();
        if (!m_tls->continue_handshake(ctx->in_buff.data(), ctx->in_buff_size, ctx->out_buff.data(), ctx->out_buff_size)) {
            end_hand_shake(false);
            return;
        }

        if ((s32)ctx->in_buff.size() == ctx->in_buff_size) {
            end_hand_shake(false);
            return;
        }

        continue_hand_shake_inner();
    });
}

void ddtls_socket_async::end_hand_shake(bool successful)
{
    ddtls_handshake_async_ctx* ctx = (ddtls_handshake_async_ctx*)(m_handshake_ctx);
    ctx->callback(successful);
    delete ctx;
    m_handshake_ctx = nullptr;
}


} // namespace NSP_DD
