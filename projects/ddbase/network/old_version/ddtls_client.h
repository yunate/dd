#ifndef ddbase_network_ddtls_client_h_
#define ddbase_network_ddtls_client_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddtls.h"

namespace NSP_DD {

class ddtls_client : public ddtls
{
public:
    ddtls_client(const std::wstring& server_name);
    ~ddtls_client();
    virtual bool continue_handshake(u8* in_buff, s32& in_buff_size, u8* out_buff, s32& out_buff_size) override;

private:
    bool acquire_credentials_handle();
    std::wstring m_server_name;
};

} // namespace NSP_DD

#endif // ddbase_network_ddtls_client_h_
