#include "ddbase/stdafx.h"
#include "ddbase/network/ddtls.h"
#include <schannel.h>

namespace NSP_DD {
static void push_back(ddbuff& src, void* buff, s32 buff_size)
{
    src.resize(src.size() + buff_size);
    ::memcpy(src.data() + src.size() - buff_size, buff, buff_size);
}

static void remain(ddbuff& src, s32 remain_size)
{
    ::memcpy(src.data(), src.data() + src.size() - remain_size, remain_size);
    src.resize(remain_size);
}

static s32 get_hand_shake_buff_size()
{
    // payload + extra over head for header/mac/padding (probably an overestimate)
    return 16384 + 512;
}

static s32 get_tls_encode_extra_size(const SecPkgContext_StreamSizes& native_size)
{
    return native_size.cbTrailer + native_size.cbHeader;
}

static s32 get_tls_encode_max_size(const SecPkgContext_StreamSizes& native_size)
{
    return native_size.cbMaximumMessage;
}
} // namespace NSP_DD

namespace NSP_DD {
#define DDSEC_SUCCESS(ss) ((ss) >= 0)

ddtls::ddtls()
{
    s32 buff_size = get_hand_shake_buff_size();
    m_handshake_ctx.in_buff.resize(buff_size);
    m_handshake_ctx.out_buff.resize(buff_size);
}

ddtls::~ddtls()
{
    if (has_cred_init()) {
        ::FreeCredentialHandle(&m_cred);
        m_cred = { 0 };
    }

    if ((m_ctxt.dwLower != 0 || m_ctxt.dwUpper != 0)) {
        ::DeleteSecurityContext(&m_ctxt);
        m_ctxt = { 0 };
    }
}

std::unique_ptr<ddtls> ddtls::create_client(const std::string& server_name)
{
    return std::unique_ptr<ddtls>(new (std::nothrow)ddtls_client(server_name));
}

void ddtls::handshake(pfn_io_opt read_opt, pfn_io_opt write_opt, const std::function<void(bool successful)>& callback, const ddasync_caller& async_caller, ddexpire expire /* = ddexpire::never */)
{
    ddmaybe_async_call(async_caller, [=]() {
        DDASSERT(read_opt != nullptr);
        DDASSERT(write_opt != nullptr);
        if (m_handshake_ctx.need_send) {
            m_handshake_ctx.need_send = false;
            write_opt(m_handshake_ctx.out_buff.data(), m_handshake_ctx.out_buff_size, [async_caller, read_opt, write_opt, expire, callback, this](bool successful, s32 byte) {
                if (!successful || byte != m_handshake_ctx.out_buff_size) {
                    on_handshake_end(false, callback);
                    return;
                }

                m_handshake_ctx.out_buff_size = 0;
                handshake(read_opt, write_opt, callback, async_caller, expire);
            }, expire);
            return;
        }

        if (m_handshake_ctx.need_recv) {
            m_handshake_ctx.need_recv = false;
            read_opt(m_handshake_ctx.in_buff.data() + m_handshake_ctx.in_buff_size,
                (s32)m_handshake_ctx.in_buff.size() - m_handshake_ctx.in_buff_size,
                [async_caller, read_opt, write_opt, expire, callback, this](bool successful, s32 byte) {
                if (!successful) {
                    on_handshake_end(false, callback);
                    return;
                }

                m_handshake_ctx.in_buff_size += byte;
                handshake(read_opt, write_opt, callback, async_caller, expire);
            }, expire);
            return;
        }

        if (m_handshake_ctx.complete) {
            on_handshake_end(true, callback);
            return;
        }

        m_handshake_ctx.out_buff_size = (s32)m_handshake_ctx.out_buff.size();
        if (!handshake_inner(m_handshake_ctx.in_buff.data(), m_handshake_ctx.in_buff_size,
            m_handshake_ctx.out_buff.data(), m_handshake_ctx.out_buff_size)) {
            on_handshake_end(false, callback);
            return;
        }

        if ((s32)m_handshake_ctx.in_buff.size() == m_handshake_ctx.in_buff_size) {
            on_handshake_end(false, callback);
            return;
        }

        if (expire.epoch <= ddtime::now_ms()) {
            dderror_code::set_last_error(dderror_code::time_out);
            on_handshake_end(false, callback);
            return;
        }

        handshake(read_opt, write_opt, callback, async_caller, expire);
    });
}

void ddtls::on_handshake_end(bool successful, const std::function<void(bool successful)>& callback)
{
    m_handshake_ctx.in_buff = std::move(std::vector<u8>());
    m_handshake_ctx.out_buff = std::move(std::vector<u8>());
    callback(successful);
}

bool ddtls::has_cred_init() const
{
    return (m_cred.dwLower != 0 || m_cred.dwUpper != 0);
}

void ddtls::query_stream_size()
{
    (void)::QueryContextAttributes(&m_ctxt, SECPKG_ATTR_STREAM_SIZES, &m_tls_ctx_max_size);
}

bool ddtls::encode(u8* in_buff, s32& encoded_size, u8* out_buff, s32& decoded_size)
{
    int use = min(encoded_size, get_tls_encode_max_size(m_tls_ctx_max_size));
    DDASSERT(get_tls_encode_extra_size(m_tls_ctx_max_size) + use <= decoded_size);
    SecBuffer buffers[3] = { 0 };
    buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
    buffers[0].pvBuffer = out_buff;
    buffers[0].cbBuffer = m_tls_ctx_max_size.cbHeader;
    buffers[1].BufferType = SECBUFFER_DATA;
    buffers[1].pvBuffer = out_buff + m_tls_ctx_max_size.cbHeader;
    buffers[1].cbBuffer = use;
    buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
    buffers[2].pvBuffer = out_buff + m_tls_ctx_max_size.cbHeader + use;
    buffers[2].cbBuffer = m_tls_ctx_max_size.cbTrailer;
    (void)::memcpy(buffers[1].pvBuffer, in_buff, use);
    SecBufferDesc buff_desc = { SECBUFFER_VERSION, ARRAYSIZE(buffers), buffers };
    SECURITY_STATUS sec = ::EncryptMessage(&m_ctxt, 0, &buff_desc, 0);
    if (sec != SEC_E_OK) {
        // this should not happen, but just in case check it
        return false;
    }
    decoded_size = buffers[0].cbBuffer + buffers[1].cbBuffer + buffers[2].cbBuffer;
    encoded_size -= use;
    return true;
}

ddtls::codec_result ddtls::encode(u8* buff, s32 buff_size)
{
    ddbuff& encode_buff = m_edcode_ctx.encode_buff;
    ddbuff& raw_buff = m_edcode_ctx.raw_buff;

    // 将新来的buff中的数据拷贝到raw_buff中
    push_back(raw_buff, buff, buff_size);
    s32 raw_size = (s32)raw_buff.size();

    // 计算加密后的数据size, 这个值比raw_buff size要大
    s32 encoded_size = get_encode_size((s32)raw_buff.size());
    encode_buff.resize(encoded_size);

    // 加密
    codec_result result;
    result.successful = encode((u8*)raw_buff.data(), raw_size, encode_buff.data(), encoded_size);
    if (!result.successful) {
        return result;
    }

    // 将raw_buff最右边剩余的未加密加密数据移动到最左边
    remain(raw_buff, raw_size);
    result.buff = encode_buff.data();
    result.buff_size = encoded_size;
    return result;
}

s32 ddtls::get_encode_size(s32 raw_size) const
{
    return min(raw_size, get_tls_encode_max_size(m_tls_ctx_max_size)) + get_tls_encode_extra_size(m_tls_ctx_max_size);
}

s32 ddtls::get_raw_size(s32 encode_size) const
{
    s32 tmp = (encode_size - get_tls_encode_extra_size(m_tls_ctx_max_size));
    return min(tmp, get_tls_encode_max_size(m_tls_ctx_max_size));
}

bool ddtls::decode(u8* in_buff, s32& encoded_size, s32& decoded_size)
{
    u8* buff = in_buff;
    decoded_size = 0;
    while (encoded_size > 0) {
        SecBuffer buffers[4] = { 0 };
        DDASSERT(m_tls_ctx_max_size.cBuffers == ARRAYSIZE(buffers));
        buffers[0].BufferType = SECBUFFER_DATA;
        buffers[0].pvBuffer = buff;
        buffers[0].cbBuffer = encoded_size;
        buffers[1].BufferType = SECBUFFER_EMPTY;
        buffers[2].BufferType = SECBUFFER_EMPTY;
        buffers[3].BufferType = SECBUFFER_EMPTY;
        SecBufferDesc buff_desc = { SECBUFFER_VERSION, ARRAYSIZE(buffers), buffers };
        SECURITY_STATUS sec = ::DecryptMessage(&m_ctxt, &buff_desc, 0, NULL);

        if (sec == SEC_E_OK) {
            DDASSERT(buffers[0].BufferType == SECBUFFER_STREAM_HEADER);
            DDASSERT(buffers[1].BufferType == SECBUFFER_DATA);
            DDASSERT(buffers[2].BufferType == SECBUFFER_STREAM_TRAILER);
            ::memmove(in_buff + decoded_size, buffers[1].pvBuffer, buffers[1].cbBuffer);
            decoded_size += buffers[1].cbBuffer;
            s32 remain = (s32)(buffers[3].BufferType == SECBUFFER_EXTRA ? buffers[3].cbBuffer : 0);
            buff += (encoded_size - remain);
            encoded_size = remain;
        } else if (sec == SEC_E_INCOMPLETE_MESSAGE) {
            // need more data
            return true;
        } else {
            return false;
        }
    }
    return true;
}

ddtls::codec_result ddtls::decode(u8* buff, s32 buff_size)
{
    ddbuff& encode_buff = m_decode_ctx.encode_buff;

    // 将encode_buff最右边上一次剩余的加密数据移动到最左边
    remain(encode_buff, m_decode_ctx.remain_size);
    m_decode_ctx.remain_size = 0;

    // 将新来的buff中的数据拷贝到encode_buff中
    push_back(encode_buff, buff, buff_size);

    // 解密
    codec_result result;
    s32 encode_size = (s32)encode_buff.size();
    s32 decode_size = encode_size;
    result.successful = decode(encode_buff.data(), encode_size, decode_size);
    if (!result.successful) {
        return result;
    }

    // 记录下本次解密后剩余的保存在encode_buff最右边的不完整的未解密数据大小
    m_decode_ctx.remain_size = encode_size;

    // 本次解密的数据保存在encode_buff最左边, 大小为decode_size
    result.buff = encode_buff.data();
    result.buff_size = decode_size;
    return result;
}
} // namespace NSP_DD

namespace NSP_DD {
ddtls_client::ddtls_client(const std::string& server_name) : m_server_name(server_name)
{
    m_handshake_ctx.complete = false;
    m_handshake_ctx.need_recv = false;
    m_handshake_ctx.need_send = false;
}

bool ddtls_client::acquire_credentials_handle()
{
    if (has_cred_init()) {
        return true;
    }

    SCHANNEL_CRED cred = { 0 };
    cred.dwVersion = SCHANNEL_CRED_VERSION;
    cred.dwFlags = SCH_USE_STRONG_CRYPTO            // use only strong crypto alogorithms
        | SCH_CRED_AUTO_CRED_VALIDATION             // automatically validate server certificate
        | SCH_CRED_NO_DEFAULT_CREDS;                // no client certificate authentication
    cred.grbitEnabledProtocols = SP_PROT_TLS1_2;    // allow only TLS v1.2

    SECURITY_STATUS ss = ::AcquireCredentialsHandle(NULL,
        UNISP_NAME, // use Schannel SSP
        SECPKG_CRED_OUTBOUND,
        NULL,
        &cred,
        NULL,
        NULL,
        &m_cred,
        NULL);

    return DDSEC_SUCCESS(ss);
}

bool ddtls_client::handshake_inner(u8* in_buff, s32& in_buff_size, u8* out_buff, s32& out_buff_size)
{
    DDASSERT_FMT(!m_handshake_ctx.complete, L"the handshake have handshake successful, do not need handshake again");
    SECURITY_STATUS ss;
    ULONG flags = ISC_REQ_USE_SUPPLIED_CREDS | ISC_REQ_CONFIDENTIALITY | ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT | ISC_REQ_STREAM;
    SecBuffer out_sec_buff[1] = { 0 };
    out_sec_buff[0].BufferType = SECBUFFER_TOKEN;
    out_sec_buff[0].pvBuffer = out_buff;
    out_sec_buff[0].cbBuffer = out_buff_size;
    SecBufferDesc out_buff_desc = { SECBUFFER_VERSION, ARRAYSIZE(out_sec_buff), out_sec_buff };
    if (!has_cred_init()) {
        in_buff_size = 0;
        if (!acquire_credentials_handle()) {
            return false;
        }
        ss = ::InitializeSecurityContextA(
            &m_cred,
            NULL,
            (SEC_CHAR*)m_server_name.data(),
            flags,
            0,
            SECURITY_NETWORK_DREP,
            NULL,
            0,
            &m_ctxt,
            &out_buff_desc,
            &flags,
            NULL);
    } else {
        SecBuffer in_sec_buff[2] = { 0 };
        in_sec_buff[0].BufferType = SECBUFFER_TOKEN;
        in_sec_buff[0].pvBuffer = in_buff;
        in_sec_buff[0].cbBuffer = in_buff_size;
        in_sec_buff[1].BufferType = SECBUFFER_EMPTY;
        SecBufferDesc in_buff_desc = { SECBUFFER_VERSION, ARRAYSIZE(in_sec_buff), in_sec_buff };
        ss = ::InitializeSecurityContext(
            &m_cred,
            &m_ctxt,
            NULL,
            flags,
            0,
            SECURITY_NETWORK_DREP,
            &in_buff_desc,
            0,
            NULL,
            &out_buff_desc,
            &flags,
            NULL);

        if (ss == SEC_E_INCOMPLETE_MESSAGE) {
            m_handshake_ctx.need_recv = true;
            return true;
        }

        if (in_sec_buff[1].BufferType == SECBUFFER_EXTRA) {
            (void)::memmove(in_buff, in_buff + in_buff_size - in_sec_buff[1].cbBuffer, in_sec_buff[1].cbBuffer);
            in_buff_size = in_sec_buff[1].cbBuffer;
        } else {
            in_buff_size = 0;
        }
    }

    // 客户端必须调用CompleteAuthToken, 然后将输出传递给服务器. 然后客户端等待返回的令牌, 并在另一次调用中将其传递给InitializeSecurityContext
    if (ss == SEC_I_COMPLETE_AND_CONTINUE || ss == SEC_I_COMPLETE_NEEDED) {
        SECURITY_STATUS s1 = ::CompleteAuthToken(&m_ctxt, &out_buff_desc);
        if (!DDSEC_SUCCESS(s1)) {
            return false;
        }
        ss = SEC_I_CONTINUE_NEEDED;
    }

    // 客户端必须将输出令牌发送到服务器并等待返回令牌. 然后在对InitializeSecurityContext的另一次调用中传递返回的令牌. 输出令牌可以为空.
    if (ss == SEC_I_CONTINUE_NEEDED) {
        out_buff_size = out_sec_buff[0].cbBuffer;
        m_handshake_ctx.need_recv = true;
        m_handshake_ctx.need_send = out_buff_size != 0;
        return true;
    }

    if (ss == SEC_I_INCOMPLETE_CREDENTIALS) {
        return false;
    }

    // 已成功初始化安全上下文. 不需要另一个InitializeSecurityContext调用. 如果函数返回输出令牌, 即 pOutput 中的SECBUFFER_TOKEN长度为非零, 则必须将令牌发送到服务器.
    if (ss == SEC_E_OK) {
        query_stream_size();
        out_buff_size = out_sec_buff[0].cbBuffer;
        m_handshake_ctx.complete = true;
        m_handshake_ctx.need_recv = false;
        m_handshake_ctx.need_send = out_buff_size != 0;
        return true;
    }

    // SEC_E_CERT_EXPIRED - certificate expired or revoked
    // SEC_E_WRONG_PRINCIPAL - bad hostname
    // SEC_E_UNTRUSTED_ROOT - cannot vertify CA chain
    // SEC_E_ILLEGAL_MESSAGE / SEC_E_ALGORITHM_MISMATCH - cannot negotiate crypto algorithms
    return false;
}
} // namespace NSP_DD
