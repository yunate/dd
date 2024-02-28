#ifndef ddbase_network_ddcert_h_
#define ddbase_network_ddcert_h_
#include "ddbase/dddef.h"
#include <memory>
#include <wincrypt.h>
#pragma comment(lib, "Crypt32.lib")

namespace NSP_DD {

class ddcert
{
    ddcert() = default;
public:
    static std::shared_ptr<ddcert> create_instance(const std::wstring& file_full_name, const std::wstring& psw);
    ~ddcert();
    s32 get_cert_counts();
    const CERT_CONTEXT* get_cert_ctx(s32 index);

private:
    std::vector<const CERT_CONTEXT*> m_cert_ctxs;
};

} // namespace NSP_DD

#endif // ddbase_network_ddcert_h_
