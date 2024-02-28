#ifndef ddbase_network_ddtls_server_h_
#define ddbase_network_ddtls_server_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddtls.h"
#include "ddbase/network/ddcert.h"

namespace NSP_DD {

class ddtls_server : public ddtls
{
public:
    ddtls_server(const std::shared_ptr<ddcert>& cert);
    ~ddtls_server();
    virtual bool continue_handshake(u8* in_buff, s32& in_buff_size, u8* out_buff, s32& out_buff_size) override;

private:
    bool acquire_credentials_handle();
    std::shared_ptr<ddcert> m_cert;
};

} // namespace NSP_DD

#endif // ddbase_network_ddtls_server_h_
