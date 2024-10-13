#ifndef UPDATER_H
#define UPDATER_H

#include <urlmon.h>
#include <string>
#include <utility>

#pragma comment(lib, "urlmon.lib")

const IID IID_IBindStatusCallback = {0x79eac9c1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}};
const IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

class DownloadProgressCallback : public IBindStatusCallback {
public:
    STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding* pib) override { return E_NOTIMPL; }
    STDMETHOD(GetPriority)(LONG* pnPriority) override { return E_NOTIMPL; }
    STDMETHOD(OnLowResource)(DWORD reserved) override { return E_NOTIMPL; }
    STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText) override;
    STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError) override { return E_NOTIMPL; }
    STDMETHOD(GetBindInfo)(DWORD* grfBINDF, BINDINFO* pbindinfo) override { return E_NOTIMPL; }
    STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC* pformatetc, STGMEDIUM* pstgmed) override { return E_NOTIMPL; }
    STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown* punk) override { return E_NOTIMPL; }

    STDMETHOD_(ULONG, AddRef)() override { return 1; }
    STDMETHOD_(ULONG, Release)() override { return 1; }
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override {
        if (riid == IID_IUnknown || riid == IID_IBindStatusCallback) {
            *ppvObject = static_cast<IBindStatusCallback*>(this);
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
};

std::pair<std::string, std::string> FetchLatestVersion();
void Log(const std::string& message);
void SelfUpdate();

#endif // UPDATER_H