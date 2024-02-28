#ifndef ddbase_windows_ddcom_utils_h_
#define ddbase_windows_ddcom_utils_h_

#include "ddbase/dddef.h"
#include "ddbase/ddnocopyable.hpp"

#include "ddbase/ddrandom.h"
#include <windows.h>
#include <wrl/client.h>

#define UUIDREG(GUID, CLASS) class __declspec(uuid(GUID)) CLASS
#define UUIDOF(CLASS) __uuidof(CLASS)
#define  DDREF_COUNT_GEN(CALL_TYPE, REF_COUNT_DEF)                                                                        \
    inline unsigned long CALL_TYPE Release()                                                                              \
    {                                                                                                                     \
        unsigned long cRef = static_cast<unsigned long>(InterlockedDecrement(reinterpret_cast<long*>(&REF_COUNT_DEF)));   \
        if (cRef == 0) {                                                                                                  \
            delete this;                                                                                                  \
        }                                                                                                                 \
        return cRef;                                                                                                      \
    }                                                                                                                     \
    inline unsigned long CALL_TYPE AddRef()                                                                               \
    {                                                                                                                     \
        InterlockedIncrement(reinterpret_cast<long*>(&REF_COUNT_DEF));                                                    \
        return REF_COUNT_DEF;                                                                                             \
    }                                                                                                                     \
    inline unsigned long CALL_TYPE RefCount()                                                                             \
    {                                                                                                                     \
        return REF_COUNT_DEF;                                                                                             \
    }                                                                                                                     \
    unsigned long REF_COUNT_DEF = 0;                                                                                      \


#define QUERY_INTERFACE(rrid, className)                           \
if (rrid == UUIDOF(className)) {                                   \
    *ppvObject = static_cast<className*>(this);                    \
    if (*ppvObject != nullptr) {                                   \
        AddRef();                                                  \
        return S_OK;                                               \
    }                                                              \
}                                                                  \

#define QUERY_INTERFACE_EX(rrid, className, toClassName)           \
if (rrid == UUIDOF(className)) {                                   \
    *ppvObject = static_cast<toClassName*>(this);                  \
    if (*ppvObject != nullptr) {                                   \
        AddRef();                                                  \
        return S_OK;                                               \
    }                                                              \
}                                                                  \

#define HR_RTN_NULL_POINTER(point)                                 \
if (point == nullptr) {                                            \
    return E_POINTER;                                              \
}                                                                  \

#define DDSAFE_RELEASE(point)                                      \
if (point != NULL) {                                               \
    point->Release();                                              \
    point = NULL;                                                  \
}                                                                  \

#define DDSAFE_DELETE(point)                                       \
if (point != nullptr) {                                            \
    delete point;                                                  \
    point = nullptr;                                               \
}                                                                  \

template<typename T>
using DDComPtr = Microsoft::WRL::ComPtr<T>;

#define DDUNUSED(param) param;
#define DDNULL_POINT_RTN(expr) { if ((expr) == NULL) { return E_POINTER; }}
#define DDHR_FAIL_RTN(expr) { auto __hr__ = (expr); if (FAILED(__hr__)) { return __hr__; } }
#define DDHR_FAIL_BRK(expr) { auto __hr__ = (expr); if (FAILED(__hr__)) { break; } }

namespace NSP_DD {
// https://docs.microsoft.com/en-us/windows/win32/com/inprocserver32?redirectedfrom=MSDN
enum class com_thread_model
{
    Apartment,  // single threaded
    Both,       // single threaded or multi threaded ***
    Free,       // multi threaded
    Neutral     // 
};

//////////////////////////////////////////////////////////////////////////
// HKEY_CLASSES_ROOT\CLSID\{clsid}
//     default desc
//     InprocServer32
//          default dllFullPath
//          ThreadingModel threadModel
//////////////////////////////////////////////////////////////////////////
HRESULT write_com_init_register(const std::wstring& clsid, const std::wstring& desc, com_thread_model threadModel, const std::wstring dllFullPath);
HRESULT write_com_uninit_register(const std::wstring& clsid);
bool com_has_register(const std::wstring& clsid); // 可以使用该函数来判断是否存在

//////////////////////////////direct show 注册表////////////////////////////////////////////
// HKEY_CLASSES_ROOT\CLSID\{categroy}\Instance\
//     {clsid} // 这个 guid 最好和 com 注册的id一致
//          CLSID {clsid} // 这个 guid 一定要和 com 注册的id一致
//          FilterData data
//          FriendlyName nickName
//////////////////////////////////////////////////////////////////////////
// struct dshow_register_desc
// {
//     std::wstring categroy;
//     std::wstring clsid;
//     ddbuff filterData; // REGFILTER2
//     std::wstring friendlyName;
// };
// HRESULT write_dshow_filter_init_register(const dshow_register_desc& desc); // 该函数可以用来注册 direct show filter 也可以用来更新它
// HRESULT write_dshow_filter_uninit_register(const std::wstring& categroy, const std::wstring& clsid);
// bool read_dshow_filter_register(dshow_register_desc& desc); // 可以使用该函数来判断是否安装

class DDRefCountBase
{
public:
    virtual ~DDRefCountBase() { }
    DDREF_COUNT_GEN(__stdcall, m_RefCount);
};

struct DDComDesc
{
    GUID comClsId;
    std::wstring comDesc;
    com_thread_model threadModel;
};

class IDDComFactory : public IClassFactory
{
public:
    IDDComFactory() { AddRef(); };
    virtual ~IDDComFactory() = default;
    IDDComFactory(const IDDComFactory&) = delete;
    IDDComFactory(IDDComFactory&&) = delete;
    IDDComFactory& operator= (const IDDComFactory&) = delete;
    IDDComFactory& operator= (IDDComFactory&&) = delete;

    // 待重载
    virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;
    virtual HRESULT RegisterCore() { return S_OK; };
    virtual HRESULT UnRegisterCore() { return S_OK; };
    virtual DDComDesc GetComDesc() = 0;

public:
    virtual HRESULT Register(const std::wstring& fullDllPath)
    {
        HRESULT hr = S_FALSE;
        do {
            if ((hr = RegisterCore()) != S_OK) {
                break;
            }

            DDComDesc comDesc = GetComDesc();
            std::wstring comClsId;
            if (!ddguid::ddguid_str(comDesc.comClsId, comClsId)) {
                break;
            }

            hr = write_com_init_register(comClsId, comDesc.comDesc, comDesc.threadModel, fullDllPath);
        } while (0);

        if (hr != S_OK) {
            (void)UnRegister();
        }
        return hr;
    }
    virtual HRESULT UnRegister()
    {
        HRESULT hr = S_FALSE;
        do {
            if ((hr = UnRegisterCore()) != S_OK) {
                break;
            }

            DDComDesc comDesc = GetComDesc();
            std::wstring comClsId;
            if (ddguid::ddguid_str(comDesc.comClsId, comClsId)) {
                break;
            }
            hr = write_com_uninit_register(comClsId);
        } while (0);
        return hr;
    }
    virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock)
    {
        if (fLock) {
            (void)InterlockedIncrement(reinterpret_cast<long*>(&m_lockCnt));
        } else {
            (void)InterlockedDecrement(reinterpret_cast<long*>(&m_lockCnt));
        }
        return S_OK;
    }
    virtual HRESULT DllGetClassObject(GUID rclsid, void** ppv)
    {
        DDComDesc comDesc = GetComDesc();
        if (rclsid == comDesc.comClsId) {
            AddRef();
            *ppv = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }
public:
    DDREF_COUNT_GEN(STDMETHODCALLTYPE, m_refCount);
    inline unsigned long GetLockCnt()
    {
        return m_lockCnt;
    }
protected:
    unsigned long m_lockCnt = 0;
};
} // namespace NSP_DD
#endif // ddbase_windows_ddcom_utils_h_
