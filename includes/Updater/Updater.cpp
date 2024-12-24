#include "Updater.h"
#include "../Utils.h"
#include <fstream>
#include <regex>
#include <comdef.h>
#include <windows.h>

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
        return;
    }

    if (latestVersion <= sylentx_version) {
        Log("No new update available.");
        return;
    }

    Log("New Sylent-X version available: " + latestVersion);

    // Download the latest version
    DownloadProgressCallback progressCallback;
    HRESULT hr = URLDownloadToFile(NULL, downloadURL.c_str(), "Sylent-X_New.exe", 0, &progressCallback);
    if (SUCCEEDED(hr)) {
        Log("Update downloaded successfully");
        MessageBox(NULL, "Update downloaded! The application will now quit to complete the update. Please restart manually after 5 seconds.", "Update", MB_OK | MB_TOPMOST);

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

std::string generateRandomString(size_t length) {
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string randomString;
    srand(static_cast<unsigned int>(time(0))); // Seed for randomness

    for (size_t i = 0; i < length; ++i) {
        randomString += characters[rand() % characters.size()];
    }
    return randomString;
}

std::pair<std::string, std::string> FetchLatestVersion() {
    std::string latestVersion;
    std::string downloadURL;
    std::string randomString = generateRandomString(10); // Generate a random string of length 10
    std::string url = "https://patch.sylent-x.com/latest_version.txt?v=" + randomString;
    LogDebug("Fetching latest version from: " + url);

    HRESULT hr = URLDownloadToFile(NULL, url.c_str(), "latest_version.txt", 0, NULL);
    if (SUCCEEDED(hr)) {
        std::ifstream versionFile("latest_version.txt");
        if (versionFile.is_open()) {
            std::getline(versionFile, latestVersion);
            std::getline(versionFile, downloadURL);
            versionFile.close();
            if (latestVersion.empty() || downloadURL.empty()) {
                Log("Error: Version file is missing version number or download URL.");
                return {"", ""};
            }

            // Check version number format
            std::regex versionRegex(R"(\d+[a-zA-Z]?\.\d+\.\d+(-[a-zA-Z]+)?)");
            if (!std::regex_match(latestVersion, versionRegex)) {
                Log("Error: Invalid version number format.");
                return {"", ""};
            }

            // Check URL format
            std::regex urlRegex(R"(https:\/\/[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,}(\/\S*)?)");
            if (!std::regex_match(downloadURL, urlRegex)) {
                Log("Error: Invalid download URL format.");
                return {"", ""};
            }
        } else {
            Log("Error: Unable to open the version file.");
        }
        LogDebug("Fetched latest version: " + latestVersion + " - Download URL: " + downloadURL);
        std::remove("latest_version.txt");
    } else {
        Log("Error: Failed to download the version file.");
    }
    return {latestVersion, downloadURL};
}