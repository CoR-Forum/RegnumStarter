#ifndef UPDATER_H
#define UPDATER_H

#include <string>
#include <urlmon.h>
#include <comdef.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <objbase.h> // Include for COM
#include "Utils.h"

#pragma comment(lib, "urlmon.lib")

void InitializePointers();

class DownloadProgressCallback : public IBindStatusCallback {
public:
    STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding* pib) { return E_NOTIMPL; }
    STDMETHOD(GetPriority)(LONG* pnPriority) { return E_NOTIMPL; }
    STDMETHOD(OnLowResource)(DWORD reserved) { return E_NOTIMPL; }
    STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText);
    STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError) { return E_NOTIMPL; }
    STDMETHOD(GetBindInfo)(DWORD* grfBINDF, BINDINFO* pbindinfo) { return E_NOTIMPL; }
    STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC* pformatetc, STGMEDIUM* pstgmed) { return E_NOTIMPL; }
    STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown* punk) { return E_NOTIMPL; }

    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) {
        if (riid == IID_IUnknown || riid == IID_IBindStatusCallback) {
            *ppvObject = static_cast<IBindStatusCallback*>(this);
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
};

void SelfUpdate();
std::pair<std::string, std::string> FetchLatestVersion();
void Log(const std::string& message);

#endif // UPDATER_H

extern const std::string currentVersion;
extern const char* appDataPath;

STDMETHODIMP DownloadProgressCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText) {
    std::string statusText = szStatusText ? WStringToString(szStatusText) : "";
    Log("Download progress: " + std::to_string(ulProgress) + "/" + std::to_string(ulProgressMax) + " - " + statusText);
    return S_OK;
}

void SelfUpdate() {
    Log("Checking for updates...");

    auto [latestVersion, downloadURL] = FetchLatestVersion();
    if (latestVersion.empty() || downloadURL.empty()) {
        Log("Failed to fetch the latest version or download URL");
        MessageBox(NULL, "Failed to fetch the latest version or download URL. This may be due to a network or server error. You can continue.", "Error", MB_ICONERROR);
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
        MessageBox(NULL, "Update downloaded! The application will now restart to complete the update.", "Update", MB_OK);

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
            MessageBox(NULL, "Failed to create batch file.", "Error", MB_ICONERROR);
        }
    } else {
        // Get the error message from HRESULT
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        Log("Failed to download update: " + std::string(errMsg));
        MessageBox(NULL, ("Failed to download update: " + std::string(errMsg)).c_str(), "Error", MB_ICONERROR);
    }
}

std::pair<std::string, std::string> FetchLatestVersion() {
    std::string latestVersion;
    std::string downloadURL;
    HRESULT hr = URLDownloadToFile(NULL, "https://cor-forum.de/regnum/sylent/latest_version.txt", "latest_version.txt", 0, NULL);
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