#include "ddbase/stdafx.h"
#include "ddbase/network/ddcert.h"
#include "ddbase/dderror_code.h"
#include "ddbase/file/ddfile.h"
#include "ddbase/file/dddir.h"

namespace NSP_DD {

std::shared_ptr<ddcert> ddcert::create_instance(const std::wstring& file_full_name, const std::wstring& psw)
{
    std::shared_ptr<ddcert> cert(new(std::nothrow) ddcert());
    if (cert == nullptr) {
        dderror_code::set_last_error(dderror_code::out_of_memory);
        return nullptr;
    }

    if (!dddir::is_path_exist(file_full_name) || dddir::is_dir(file_full_name)) {
        return nullptr;
    }

    std::unique_ptr<ddfile> file(ddfile::create_utf8_file(file_full_name));
    if (file == nullptr) {
        return nullptr;
    }

    s32 file_size = (s32)file->file_size();
    std::vector<u8> buff(file_size);
    if (file->read(buff.data(), file_size) != file_size) {
        return nullptr;
    }

    CRYPT_DATA_BLOB pfx = { 0 };
    pfx.cbData = (DWORD)file_size;
    pfx.pbData = (BYTE*)buff.data();

    HCERTSTORE cert_store = ::PFXImportCertStore(&pfx, psw.c_str(), 0);
    if (cert_store == NULL) {
        return nullptr;
    }

    PCCERT_CONTEXT ctx = ::CertEnumCertificatesInStore(cert_store, nullptr);
    while (ctx != NULL) {
        cert->m_cert_ctxs.push_back(::CertDuplicateCertificateContext(ctx));
        ctx = ::CertEnumCertificatesInStore(cert_store, ctx);
    }

    cert->m_cert_ctxs.shrink_to_fit();
    ::CertCloseStore(cert_store, 0);
    return cert;
}

ddcert::~ddcert()
{
    for (auto it : m_cert_ctxs) {
        ::CertFreeCertificateContext(it);
    }
    m_cert_ctxs.clear();
}

s32 ddcert::get_cert_counts()
{
    return (s32)m_cert_ctxs.size();
}

const CERT_CONTEXT* ddcert::get_cert_ctx(s32 index)
{
    if (index >= get_cert_counts()) {
        return NULL;
    }
    return m_cert_ctxs[index];
}
} // namespace NSP_DD
