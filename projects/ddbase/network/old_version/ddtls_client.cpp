#include "ddbase/stdafx.h"
#include "ddbase/network/ddtls_client.h"
#include <schannel.h>


namespace NSP_DD {

ddtls_client::~ddtls_client()
{
}

ddtls_client::ddtls_client(const std::wstring& server_name) :
    m_server_name(server_name)
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

bool ddtls_client::continue_handshake(u8* in_buff, s32& in_buff_size, u8* out_buff, s32& out_buff_size)
{
    DDASSERT_FMT(!m_handshake_ctx.complete, L"the handshake have handshake successful, do not need handshake again");
    SECURITY_STATUS ss;
    ULONG flags = ISC_REQ_USE_SUPPLIED_CREDS | ISC_REQ_CONFIDENTIALITY | ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT | ISC_REQ_STREAM;
    SecBuffer out_sec_buff[1] = {0};
    out_sec_buff[0].BufferType = SECBUFFER_TOKEN;
    out_sec_buff[0].pvBuffer = out_buff;
    out_sec_buff[0].cbBuffer = out_buff_size;
    SecBufferDesc out_buff_desc = { SECBUFFER_VERSION, ARRAYSIZE(out_sec_buff), out_sec_buff };
    if (!has_cred_init()) {
        in_buff_size = 0;
        if (!acquire_credentials_handle()) {
            return false;
        }
        ss = ::InitializeSecurityContext(
            &m_cred,
            NULL,
            (SEC_WCHAR*)m_server_name.data(),
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
