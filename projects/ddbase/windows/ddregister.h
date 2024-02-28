#ifndef ddbase_windows_ddregister_h_
#define ddbase_windows_ddregister_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"
#include <windows.h>
namespace NSP_DD {

class DDHKEY
{
    DDNO_COPY_MOVE(DDHKEY);
public:
    DDHKEY() = default;
    DDHKEY& operator=(HKEY key)
    {
        if (m_key != NULL) {
            ::RegCloseKey(m_key);
            m_key = NULL;
        }
        m_key = key;
    }
    operator HKEY()
    {
        return m_key;
    }
    HKEY* operator&()
    {
        return &m_key;
    }
    ~DDHKEY()
    {
        if (m_key != NULL) {
            ::RegCloseKey(m_key);
            m_key = NULL;
        }
    }

    HKEY m_key = NULL;
};

} // namespace NSP_DD
#endif // ddbase_windows_ddregister_h_