#include "ApiHandler.h"
#include "../InternetUtils/InternetUtils.h"

void ModifyGlobalSettings(const std::string& settingName, const std::string& settingValue) {
    try {
        std::string path = "/admin.php?action=modifyGlobalSettings&username=" + login + "&password=" + password + "&setting=" + settingName + "&value=" + settingValue;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("System setting updated successfully: " + message);
            MessageBox(NULL, message.c_str(), "Success", MB_ICONINFORMATION | MB_TOPMOST);
        } else {
            LogDebug("Failed to update system setting: " + message);
            MessageBox(NULL, message.c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
        MessageBox(NULL, e.what(), "Exception", MB_ICONERROR | MB_TOPMOST);
    }
}


void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime) {
    try {
        // Check if licensedFeatures is provided
        if (licensedFeatures.empty()) {
            LogDebug("licensedFeatures is empty. Cannot generate license.");
            MessageBox(NULL, "licensedFeatures is empty. Cannot generate license.", "Error", MB_ICONERROR | MB_TOPMOST);
            return;
        }

        // Split licensedFeatures by commas and create a JSON array
        std::vector<std::string> features;
        std::stringstream ss(licensedFeatures);
        std::string feature;
        while (std::getline(ss, feature, ',')) {
            features.push_back(feature);
        }
        nlohmann::json jsonFeatures = features;

        std::string path = "/admin.php?action=generateLicenseKey&username=" + login + "&password=" + password + "&licensedFeatures=" + jsonFeatures.dump() + "&runtime=" + runtime;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        // Split the response into individual JSON objects
        std::vector<std::string> jsonResponses;
        size_t pos = 0;
        while ((pos = response.find("}{")) != std::string::npos) {
            jsonResponses.push_back(response.substr(0, pos + 1));
            response.erase(0, pos + 1);
        }
        jsonResponses.push_back(response);

        for (const auto& jsonResponseStr : jsonResponses) {
            auto jsonResponse = nlohmann::json::parse(jsonResponseStr);
            std::string status = jsonResponse["status"];
            std::string message = jsonResponse.contains("message") ? jsonResponse["message"] : "";

            if (status == "success") {
                generated_license_key = jsonResponse["licenseKey"]["license_key"]; // Set the global variable
                LogDebug("License generated successfully: " + generated_license_key);
                GetAllLicenses();
                return; // Exit the function after successfully generating the license
            } else {
                LogDebug("Failed to generate license: " + message);
                MessageBox(NULL, ("Failed to generate license: " + message).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
            }
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
        MessageBox(NULL, e.what(), "Exception", MB_ICONERROR | MB_TOPMOST);
    }
}

void ExpireLicense(int licenseId) {
    try {
        std::string path = "/admin.php?action=expireLicense&username=" + login + "&password=" + password + "&licenseId=" + std::to_string(licenseId);
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("License expired successfully: " + message);
            MessageBox(NULL, message.c_str(), "Success", MB_ICONINFORMATION | MB_TOPMOST);
            GetAllLicenses();
        } else {
            LogDebug("Failed to expire license: " + message);
            MessageBox(NULL, message.c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
            GetAllLicenses();
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
        MessageBox(NULL, e.what(), "Exception", MB_ICONERROR | MB_TOPMOST);
        GetAllLicenses();
    }
}


void GetAllUsers() {
    try {
        std::string path = "/admin.php?action=getUsers&username=" + login + "&password=" + password;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        // Store the raw JSON response in the global variable
        GetAllUsersRawJson = response;
        LogDebug("Users fetched successfully: " + response);

    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
    }
}

void GetAllLicenses() {
    try {
        std::string path = "/admin.php?action=getLicenses&username=" + login + "&password=" + password;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        // Store the raw JSON response in the global variable
        GetAllLicensesRawJson = response;
        LogDebug("Licenses fetched successfully: " + response);

    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
    }
}

void ToggleUserBan(int userId) {
    try {
        std::string path = "/admin.php?action=toggleUserBan&username=" + login + "&password=" + password + "&userId=" + std::to_string(userId);
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("User ban toggled successfully: " + message);
            GetAllUsers();
        } else {
            LogDebug("Failed to toggle user ban: " + message);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
        GetAllUsers();
    }
}

void ToggleUserAdmin(int userId) {
    try {
        std::string path = "/admin.php?action=toggleUserAdmin&username=" + login + "&password=" + password + "&userId=" + std::to_string(userId);
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("User admin status toggled successfully: " + message);
            GetAllUsers();
        } else {
            LogDebug("Failed to toggle user admin status: " + message);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
        GetAllUsers();
    }
}

void ToggleUserActivation(int userId) {
    try {
        std::string path = "/admin.php?action=toggleUserActivation&username=" + login + "&password=" + password + "&userId=" + std::to_string(userId);
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("User activation status toggled successfully: " + message);
            GetAllUsers();
        } else {
            LogDebug("Failed to toggle user activation status: " + message);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
        GetAllUsers();
    }
}