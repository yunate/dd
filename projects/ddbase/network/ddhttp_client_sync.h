#ifndef ddbase_network_ddhttp_client_sync_h_
#define ddbase_network_ddhttp_client_sync_h_
#include "ddbase/dddef.h"
#include "ddbase/network/ddsocket_sync.h"
#include "ddbase/network/ddhttp_data_parser.h"
#include "ddbase/stream/ddistream.h"

namespace NSP_DD {

class ddhttp_client_sync
{
    ddhttp_client_sync() = default;
public:
    static std::unique_ptr<ddhttp_client_sync> create_instance(std::shared_ptr<ddsocket_sync> socket);
    ~ddhttp_client_sync() = default;

    bool send_header(const ddhttp_request_header& request_header);
    bool send_body(const u8* buff, s32 buff_size);
    bool send_body(ddistream* stream);

    // @return nullptr 表示失败
    const ddhttp_response_parse::context* recv();

private:
    std::shared_ptr<ddsocket_sync> m_socket;

    ddhttp_response_parse m_parser;
    const ddhttp_response_parse::context* m_parse_ctx = nullptr;
    std::vector<u8> m_buff;
    s32 m_buff_remain_size = 0;
};

} // namespace NSP_DD
#endif // ddbase_network_ddhttp_client_sync_h_
