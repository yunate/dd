#ifndef ddbase_network_ddtls_h_
#define ddbase_network_ddtls_h_
#include "ddbase/dddef.h"
#include "ddbase/thread/ddasync.h"
#include "ddbase/coroutine/ddcoroutine.h"
#include "ddbase/network/ddcert.h"

#include <tuple>

#pragma comment(lib, "Secur32.lib")
#define SECURITY_WIN32
#include <Sspi.h>

namespace NSP_DD {

class ddtls
{
public:
    static std::shared_ptr<ddtls> create_client(const std::string& server_name);
    static std::shared_ptr<ddtls> create_server(const std::shared_ptr<ddcert>& cert);

public:
    ddtls();
    virtual ~ddtls();

    // return: std::tuple<bool, s32>
    //   bool: successful or not
    //   s32: io bytes
    using pfn_io_opt = std::function<ddcoroutine<std::tuple<bool, s32>>(void* buff, s32 buff_size, ddexpire expire)>;
    ddcoroutine<bool> handshake(const pfn_io_opt& read_opt, const pfn_io_opt& write_opt, ddexpire expire = ddexpire::never);

    struct codec_result
    {
        bool successful = false;

        // 内部维护内存, 调用者不该修改和释放
        const u8* buff = nullptr;
        s32 buff_size = 0;
    };

    // buff 原始数据, 如果buff中的数据有剩余或者不够一次操作, 内部会将其缓存, 调用者可以放心释放内存
    codec_result encode(u8* buff, s32 buff_size);
    codec_result decode(u8* buff, s32 buff_size);

    // @param[in] in_buff 作为输入存储明文
    // @param[in, out] encoded_size 作为输入表示in_buff中的明文长度,作为输出表示加密后剩余(不完整的,需要更多数据)明文的长度
    // @param[out] out_buff 加密后密文存储buff
    // @param[out] decoded_size 加密后密文长度, 至少为get_encode_size
    // 加密前in_buff
    // --------------------------------------------------------------
    // |****************************明文*****************************|
    // --------------------------------------------------------------
    // 加密后in_buff
    // --------------------------------------------------------------
    // |****************************使用了的*****************|***剩余**|
    // --------------------------------------------------------------
    bool encode(u8* in_buff, s32& encoded_size, u8* out_buff, s32& decoded_size);

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
    bool decode(u8* in_buff, s32& encoded_size, s32& decoded_size);

    // encode 后的的密文大小会大于原文大小(多出来get_tls_encode_extra_size), 但是不会超过get_encode_size函数返回的大小
    s32 get_encode_size(s32 raw_size) const;
    s32 get_raw_size(s32 encode_size) const;

protected:
    // @param[in out] in_buff_size: 作为输入表示in_buff中的有效字符数,作为输出表示in_buff使用后剩余的字符
    // @param[in out] out_buff_size: 作为输入表示out_buff容量,作为输出表示out_buff中有多少字符
    virtual bool handshake_inner(u8* in_buff, s32& in_buff_size, u8* out_buff, s32& out_buff_size) = 0;

    bool has_cred_init() const;
    void query_stream_size();
    CredHandle m_cred = { 0 };
    CtxtHandle m_ctxt = { 0 };
    SecPkgContext_StreamSizes m_tls_ctx_max_size = { 0 };

    struct ddtls_handshake_ctx {
        // 为true表示握手结束
        bool complete = false;
        // 为true表示需要接受数据
        bool need_send = false;
        // 为true表示需要发送多数据
        bool need_recv = false;

        std::vector<u8> out_buff;
        std::vector<u8> in_buff;
        s32 in_buff_size = 0;
        s32 out_buff_size = 0;
    } m_handshake_ctx;

    struct encode_ctx
    {
        // 未加密数据
        ddbuff raw_buff;
        // 已经加密数据
        ddbuff encode_buff;
    };
    encode_ctx m_edcode_ctx;

    struct decode_ctx
    {
        // 已经加密数据
        ddbuff encode_buff;
        // encode_buff 中剩余的数据
        // |****************************明文***************|0000|***剩余**|
        s32 remain_size = 0;
    };
    decode_ctx m_decode_ctx;
};
} // namespace NSP_DD

#endif // ddbase_network_ddtls_h_