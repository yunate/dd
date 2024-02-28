#ifndef ddbase_network_ddtls_h_
#define ddbase_network_ddtls_h_
#include "ddbase/dddef.h"
#pragma comment(lib, "Secur32.lib")

#define SECURITY_WIN32
#define ISSP_MODE 1
#include <Sspi.h>
#define DDSEC_SUCCESS(Status) ((Status) >= 0)
namespace NSP_DD {

struct ddtls_handshake_ctx {
    // 为true表示握手结束
    bool complete = false;
    // 为true表示需要接受数据
    bool need_send = false;
    // 为true表示需要发送多数据
    bool need_recv = false;
};

class ddtls
{
public:
    virtual ~ddtls();

    // 获得握手阶段的buffsize
    s32 get_hand_shake_buff_size();

    const SecPkgContext_StreamSizes& get_tls_ctx_max_size();

    // 获得encode所需要的额外buff的大小
    s32 get_tls_encode_extra_size();

    // 获得一次encode的最大字符数量
    s32 get_tls_encode_max_size();

    ddtls_handshake_ctx* get_handshanke_ctx();
    // @param[in out] in_buff_size: 作为输入表示in_buff中的有效字符数,作为输出表示in_buff使用后剩余的字符
    // @param[in out] out_buff_size: 作为输入表示out_buff容量,作为输出表示out_buff中有多少字符
    virtual bool continue_handshake(u8* in_buff, s32& in_buff_size, u8* out_buff, s32& out_buff_size) = 0;

    // @param[in] in_buff 作为输入存储明文
    // @param[in, out] encoded_size 作为输入表示in_buff中的明文长度,作为输出表示加密后剩余(不完整的,需要更多数据)明文的长度
    // @param[out] out_buff 加密后密文存储buff
    // @param[out] decoded_size 加密后密文长度, 至少为encoded_size + get_tls_encode_extra_size
    // 加密前in_buff
    // --------------------------------------------------------------
    // |****************************明文*****************************|
    // --------------------------------------------------------------
    // 加密后in_buff
    // --------------------------------------------------------------
    // |****************************使用了的*****************|***剩余**|
    // --------------------------------------------------------------
    virtual bool encode(u8* in_buff, s32& encoded_size, u8* out_buff, s32& decoded_size);

    // @param[in, out] in_buff 作为输入存储密文; 作为输出, 前半段表示明文长度,其大小为decoded_size,后半段表示剩余未解密的长度其大小为encoded_size
    // @param[in, out] encoded_size 作为输入表示in_buff中的密文长度,作为输出表示解密后剩余(不完整的,需要更多数据)密文的长度
    // @param[out] decoded_size 解密后明文长度
    // 解密前in_buff
    // --------------------------------------------------------------
    // |****************************密文*****************************|
    // --------------------------------------------------------------
    // 解密后in_buff
    // --------------------------------------------------------------
    // |****************************明文***************|0000|***剩余**|
    // --------------------------------------------------------------
    virtual bool decode(u8* in_buff, s32& encoded_size, s32& decoded_size);

protected:
    bool has_cred_init();
    void query_stream_size();
    CredHandle m_cred = { 0 };
    CtxtHandle m_ctxt = { 0 };
    SecPkgContext_StreamSizes m_tls_ctx_max_size = { 0 };
    ddtls_handshake_ctx m_handshake_ctx;
};

} // namespace NSP_DD

#endif // ddbase_network_ddtls_h_
