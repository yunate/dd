#include "ddbase/stdafx.h"
#include "ddbase/network/ddtls_server.h"
#include <schannel.h>


namespace NSP_DD {
ddtls_server::ddtls_server(const std::shared_ptr<ddcert>& cert) :
    m_cert(cert)
{
    DDASSERT(m_cert != nullptr);
    m_handshake_ctx.complete = false;
    m_handshake_ctx.need_recv = true;
    m_handshake_ctx.need_send = false;
}

ddtls_server::~ddtls_server()
{
}

bool ddtls_server::continue_handshake(u8* in_buff, s32& in_buff_size, u8* out_buff, s32& out_buff_size)
{
    DDASSERT_FMT(!m_handshake_ctx.complete, L"the handshake have handshake successful, do not need handshake again");
    SECURITY_STATUS ss;
    DWORD flags = ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY | ASC_REQ_EXTENDED_ERROR | ASC_REQ_STREAM;
    SecBuffer out_sec_buff[1] = { 0 };
    out_sec_buff[0].BufferType = SECBUFFER_TOKEN;
    out_sec_buff[0].pvBuffer = out_buff;
    out_sec_buff[0].cbBuffer = out_buff_size;
    SecBufferDesc out_buff_desc = { SECBUFFER_VERSION, ARRAYSIZE(out_sec_buff), out_sec_buff };

    SecBuffer in_sec_buff[2] = { 0 };
    in_sec_buff[0].BufferType = SECBUFFER_TOKEN;
    in_sec_buff[0].pvBuffer = in_buff;
    in_sec_buff[0].cbBuffer = in_buff_size;
    in_sec_buff[1].BufferType = SECBUFFER_EMPTY;
    SecBufferDesc in_buff_desc = { SECBUFFER_VERSION, ARRAYSIZE(in_sec_buff), in_sec_buff };

    if (!has_cred_init()) {
        if (!acquire_credentials_handle()) {
            return false;
        }

        ss = ::AcceptSecurityContext(
            &m_cred,
            NULL,
            &in_buff_desc,
            flags,
            0,
            &m_ctxt,
            &out_buff_desc,
            &flags,
            NULL);
    } else {
        ss = ::AcceptSecurityContext(
            &m_cred,
            &m_ctxt,
            &in_buff_desc,
            flags,
            0,
            NULL,
            &out_buff_desc,
            &flags,
            NULL);
    }

    // 函数成功. 输入缓冲区中的数据不完整. 应用程序必须从客户端读取其他数据, 并再次调用 AcceptSecurityContext.
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

    // 函数成功. 已接受从客户端接收的安全上下文. 如果函数生成了输出令牌，则必须将令牌发送到客户端进程.
    if (ss == SEC_E_OK) {
        query_stream_size();
        out_buff_size = out_sec_buff[0].cbBuffer;
        m_handshake_ctx.complete = true;
        m_handshake_ctx.need_recv = false;
        m_handshake_ctx.need_send = out_buff_size != 0;
        return true;
    }

    // 函数成功. 服务器必须调用CompleteAuthToken并将输出令牌传递给客户端. 然后服务器必须等待客户端的返回令牌, 然后才能对 AcceptSecurityContext 进行另一次调用.
    if (ss == SEC_I_COMPLETE_AND_CONTINUE || ss == SEC_I_COMPLETE_NEEDED) {
        SECURITY_STATUS s1 = ::CompleteAuthToken(&m_ctxt, &out_buff_desc);
        if (!DDSEC_SUCCESS(s1)) {
            return false;
        }
        ss = SEC_I_CONTINUE_NEEDED;
    }

    // 函数成功. 服务器必须将输出令牌发送到客户端, 并等待返回的令牌. 返回的令牌应在 pInput 中传递, 以便再次调用 AcceptSecurityContext.
    if (ss == SEC_I_CONTINUE_NEEDED) {
        out_buff_size = out_sec_buff[0].cbBuffer;
        m_handshake_ctx.need_recv = true;
        m_handshake_ctx.need_send = out_buff_size != 0;
        return true;
    }

    return false;
}

bool ddtls_server::acquire_credentials_handle()
{
    if (has_cred_init()) {
        return true;
    }

    if (m_cert->get_cert_counts() == 0) {
        return false;
    }

    auto cert_ctx = m_cert->get_cert_ctx(0);
    SCHANNEL_CRED cred = { 0 };
    cred.dwVersion = SCHANNEL_CRED_VERSION;
    cred.cCreds = 1;
    cred.paCred = &cert_ctx;
    cred.grbitEnabledProtocols = SP_PROT_TLS1_2_SERVER;
    cred.dwFlags = SCH_USE_STRONG_CRYPTO;
    SECURITY_STATUS ss = ::AcquireCredentialsHandle(NULL,
        UNISP_NAME, // use Schannel SSP
        SECPKG_CRED_INBOUND,
        NULL,
        &cred,
        NULL,
        NULL,
        &m_cred, NULL);
    return DDSEC_SUCCESS(ss);
}

} // namespace NSP_DD
