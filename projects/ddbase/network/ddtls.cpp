#include "ddbase/stdafx.h"
#include "ddbase/network/ddtls.h"
#include <schannel.h>

namespace NSP_DD {

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

s32 ddtls::get_hand_shake_buff_size()
{
    // payload + extra over head for header/mac/padding (probably an overestimate)
    return 16384 + 512;
}

const SecPkgContext_StreamSizes& ddtls::get_tls_ctx_max_size()
{
    return m_tls_ctx_max_size;
}

s32 ddtls::get_tls_encode_extra_size()
{
    return m_tls_ctx_max_size.cbTrailer + m_tls_ctx_max_size.cbHeader;
}

s32 ddtls::get_tls_encode_max_size()
{
    return m_tls_ctx_max_size.cbMaximumMessage;
}

ddtls_handshake_ctx* ddtls::get_handshanke_ctx()
{
    return &m_handshake_ctx;
}

bool ddtls::has_cred_init()
{
    return (m_cred.dwLower != 0 || m_cred.dwUpper != 0);
}

void ddtls::query_stream_size()
{
    (void)::QueryContextAttributes(&m_ctxt, SECPKG_ATTR_STREAM_SIZES, &m_tls_ctx_max_size);
}

bool ddtls::encode(u8* in_buff, s32& encoded_size, u8* out_buff, s32& decoded_size)
{
    int use = min(encoded_size, get_tls_encode_max_size());
    DDASSERT(get_tls_encode_extra_size() + use <= decoded_size);
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

} // namespace NSP_DD
