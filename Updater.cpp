#ifndef UPDATER_H
#define UPDATER_H

#include "Utils.h"

#pragma comment(lib, "urlmon.lib")

const IID IID_IBindStatusCallback = {0x79eac9c1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}};
const IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

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

#endif // UPDATER_H

STDMETHODIMP DownloadProgressCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText) {
    std::string statusText = szStatusText ? WStringToString(szStatusText) : "";
    Log("Download progress: " + std::to_string(ulProgress) + "/" + std::to_string(ulProgressMax) + " - " + statusText);
    return S_OK;
}

void SelfUpdate() {
    auto [latestVersion, downloadURL] = FetchLatestVersion();
    if (latestVersion.empty() || downloadURL.empty()) {
        Log("Failed to fetch the latest version or download URL");
        MessageBox(NULL, "Failed to fetch the latest version or download URL. This may be due to a network or server error. You can continue.", "Error", MB_ICONERROR | MB_TOPMOST);
        InitializePointers();
        return;
    }

    if (latestVersion <= currentVersion) {
        Log("No new update available. Server returned version: " + latestVersion);
        InitializePointers();
        return;
    }

    Log("New Sylent-X version available: " + latestVersion);

    // Download the latest version
    DownloadProgressCallback progressCallback;
    HRESULT hr = URLDownloadToFile(NULL, downloadURL.c_str(), "Sylent-X_New.exe", 0, &progressCallback);
    if (SUCCEEDED(hr)) {
        Log("Update downloaded successfully");
        MessageBox(NULL, "Update downloaded! The application will now restart to complete the update.", "Update", MB_OK | MB_TOPMOST);

        // Get the name of the currently running executable
        char currentExePath[MAX_PATH];
        GetModuleFileName(NULL, currentExePath, MAX_PATH);
        std::string currentExeName = std::string(currentExePath).substr(std::string(currentExePath).find_last_of("\\/") + 1);

        // Create a batch file to replace the old executable with the new one
        std::ofstream batchFile("update.bat");
        if (batchFile.is_open()) {
            Log("Creating update batch file");
            batchFile << "@echo off\n";
            batchFile << "timeout /t 2 /nobreak\n"; // Wait for 2 seconds to ensure the application has exited
            batchFile << "move /Y Sylent-X_New.exe " << currentExeName << "\n"; // Replace the old executable
            batchFile << "start " << currentExeName << "\n"; // Restart the application
            batchFile << "del %0\n"; // Delete the batch file itself
            batchFile.close();
            Log("Update Batch file created successfully");

            // Execute the batch file
            Log("Executing update batch file");
            ShellExecute(NULL, "open", "update.bat", NULL, NULL, SW_HIDE);

            // Exit the current application
            Log("Exiting Sylent-X for update");
            PostQuitMessage(0);
        } else {
            Log("Failed to create update batch file");
            MessageBox(NULL, "Failed to create batch file.", "Error", MB_ICONERROR | MB_TOPMOST);
        }
    } else {
        // Get the error message from HRESULT
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        Log("Failed to download update: " + std::string(errMsg));
        MessageBox(NULL, ("Failed to download update: " + std::string(errMsg)).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
    }
}

std::pair<std::string, std::string> FetchLatestVersion() {
    std::string latestVersion;
    std::string downloadURL;
    HRESULT hr = URLDownloadToFile(NULL, "https://patch.sylent-x.com/v0/", "latest_version.txt", 0, NULL);
    if (SUCCEEDED(hr)) {
        std::ifstream versionFile("latest_version.txt");
        if (versionFile.is_open()) {
            std::getline(versionFile, latestVersion);
            std::getline(versionFile, downloadURL);
            versionFile.close();
        }
        std::remove("latest_version.txt");
    }
    return {latestVersion, downloadURL};
}