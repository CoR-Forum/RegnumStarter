#include "includes/Utils.h"
#include <stdexcept>
#include <sstream>

#pragma once //added for register in sylent-x.cpp maybe check it 

#define WM_CLOSE_REGISTRATION_WINDOW (WM_USER + 1)

extern HWND hwnd; // Declare the handle to the main window

extern bool featureZoom;
extern bool featureGravity;
extern bool featureMoonjump;
extern bool featureMoonwalk;

std::string login;
std::string password;

void CloseInternetHandles(HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet) {
    if (hRequest) InternetCloseHandle(hRequest);
    if (hConnect) InternetCloseHandle(hConnect);
    if (hInternet) InternetCloseHandle(hInternet);
}

bool LogAndCloseHandles(const std::string& message, HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet) {
    Log(message);
    CloseInternetHandles(hRequest, hConnect, hInternet);
    return false;
}

HINTERNET OpenInternetConnection() {
    HINTERNET hInternet = InternetOpen("Sylent-X", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) throw std::runtime_error("Failed to open internet connection");
    return hInternet;
}

HINTERNET ConnectToAPI(HINTERNET hInternet) {
    HINTERNET hConnect = InternetConnect(hInternet, "api.sylent-x.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) throw std::runtime_error("Failed to connect to API");
    return hConnect;
}

HINTERNET SendHTTPRequest(HINTERNET hConnect, const std::string& path) {
    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", path.c_str(), NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE, 0);
    if (!hRequest || !HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
        throw std::runtime_error("Failed to open or send HTTP request");
    }
    return hRequest;
}

std::string ReadResponse(HINTERNET hRequest) {
    char buffer[4096];
    DWORD bytesRead;
    std::string response;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
        response.append(buffer, bytesRead);
    }
    return response;
}

bool Login(const std::string& login, const std::string& password) {
    try {
        std::string path = "/user.php?action=login&username=" + login + "&password=" + password;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];

        if (status == "success") {
            LogDebug("User " + login + " logged in successfully");

            auto licensedFeatures = jsonResponse["licensed_features"];
            featureZoom = std::find(licensedFeatures.begin(), licensedFeatures.end(), "zoom") != licensedFeatures.end();
            featureGravity = std::find(licensedFeatures.begin(), licensedFeatures.end(), "gravity") != licensedFeatures.end();
            featureMoonjump = std::find(licensedFeatures.begin(), licensedFeatures.end(), "moonjump") != licensedFeatures.end();
            featureMoonwalk = std::find(licensedFeatures.begin(), licensedFeatures.end(), "moonwalk") != licensedFeatures.end();
            featureFov = std::find(licensedFeatures.begin(), licensedFeatures.end(), "fov") != licensedFeatures.end();

            Log("Licensed features: " + std::string(featureZoom ? "Zoom" : "") + 
                std::string(featureGravity ? ", Gravity" : "") + 
                std::string(featureMoonjump ? ", Moonjump" : ""));
                std::string(featureMoonwalk ? ", Moonwalk" : "");
                std::string(featureFov ? ", Field of View" : "");
                
            // Parse role and set isAdmin
            std::string role = jsonResponse["role"];
            isAdmin = (role == "admin");
            Log("Role: " + role);

            g_pointers = InitializePointers();
            GetMagnatCurrency();

            // Start the CheckChatMessages process in a new thread
            std::thread chatThread(CheckChatMessages);
            chatThread.detach(); // Detach the thread to run independently

            return true;
        } else {
            std::string message = jsonResponse["message"];
            Log("Failed to log in: " + message);
            return false;
        }
    } catch (const std::exception& e) {
        Log(e.what());
        return false;
    }
}

void Logout() {
    login.clear();
    password.clear();

    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\config.txt";
    std::ifstream configFile(configFilePath);
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(configFile, line)) {
        if (line.find("login=") == std::string::npos && line.find("password=") == std::string::npos) {
            lines.push_back(line);
        }
    }
    configFile.close();

    std::ofstream outFile(configFilePath);
    for (const auto& l : lines) {
        outFile << l << std::endl;
    }
    outFile.close();
    
    PostQuitMessage(0);
}

bool ResetPasswordRequest(const std::string& email) {
    try {
        std::string path = "/user.php?action=reset&resetAction=init&email=" + email;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            MessageBox(NULL, message.c_str(), "Success", MB_ICONINFORMATION | MB_TOPMOST);
            return true;
        } else {
            MessageBox(NULL, ("Failed to send password reset e-mail: " + message).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
            return false;
        }
    } catch (const std::exception& e) {
        MessageBox(NULL, e.what(), "Exception", MB_ICONERROR | MB_TOPMOST);
        return false;
    }
}

bool SetNewPassword(const std::string& token, const std::string& password) {
    try {
        std::string path = "/user.php?action=reset&resetAction=reset&token=" + token + "&password=" + password;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            MessageBox(NULL, message.c_str(), "Success", MB_ICONINFORMATION | MB_TOPMOST);
            return true;
        } else {
            MessageBox(NULL, ("Failed to set new password: " + message).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
            return false;
        }
    } catch (const std::exception& e) {
        MessageBox(NULL, e.what(), "Exception", MB_ICONERROR | MB_TOPMOST);
        return false;
    }
}

void LoadLoginCredentials(HINSTANCE hInstance) {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\config.txt";

    std::ifstream file(configFilePath);
    bool loginFound = false;
    bool passwordFound = false;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("login=") != std::string::npos) {
                login = line.substr(line.find("=") + 1);
                loginFound = true;
            }
            if (line.find("password=") != std::string::npos) {
                password = line.substr(line.find("=") + 1);
                passwordFound = true;
            }
        }
        file.close();
    } else {
        Log("Failed to open config file for reading");
    }

    if (!loginFound || !passwordFound) {
        Log("Login or password not found in config file. Please login.");
    } 
}

void SaveLoginCredentials(const std::string& login, const std::string& password) {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\config.txt";

    std::ofstream file(configFilePath);
    if (file.is_open()) {
        file << "login=" << login << std::endl;
        file << "password=" << password << std::endl;
        file.close();

        if (Login(login, password)) {
            MessageBox(NULL, "Login successful after saving credentials - Please restart the application to apply your license", "Success", MB_ICONINFORMATION | MB_TOPMOST);
            PostQuitMessage(0);
        } else {
            MessageBox(NULL, "Login failed after saving credentials", "Error", MB_ICONERROR | MB_TOPMOST);
        }
    } else {
        MessageBox(NULL, "Failed to open config file for writing", "Error", MB_ICONERROR | MB_TOPMOST);
    }
}

void SaveSettings() {
    try {
        nlohmann::json settingsJson;
        settingsJson["optionGravity"] = optionGravity;
        settingsJson["optionMoonjump"] = optionMoonjump;
        settingsJson["optionZoom"] = optionZoom;
        settingsJson["optionMoonwalk"] = optionMoonwalk;
        settingsJson["debugLog"] = debugLog;
        settingsJson["textColor"] = { textColor.x, textColor.y, textColor.z, textColor.w };

        std::string settingsStr = settingsJson.dump();
        std::string path = "/user.php?action=saveSettings&username=" + login + "&password=" + password + "&settings=" + settingsStr;

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            Log("Settings saved successfully");
        } else {
            Log("Failed to save settings: " + message);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
    }
}

void LoadSettings() {
    try {
        std::string path = "/user.php?action=loadSettings&username=" + login + "&password=" + password;

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        
        if (jsonResponse.contains("status") && jsonResponse["status"].is_string()) {
            std::string status = jsonResponse["status"];
            std::string message = jsonResponse.value("message", "");

            if (status == "success") {
                if (jsonResponse.contains("settings") && jsonResponse["settings"].is_object()) {
                    auto settingsJson = jsonResponse["settings"];
                    
                    optionGravity = settingsJson.value("optionGravity", false);
                    optionMoonjump = settingsJson.value("optionMoonjump", false);
                    optionZoom = settingsJson.value("optionZoom", false);
                    optionMoonwalk = settingsJson.value("optionMoonwalk", false);
                    debugLog = settingsJson.value("debugLog", false);

                    if (settingsJson.contains("textColor") && settingsJson["textColor"].is_array() && settingsJson["textColor"].size() == 4) {
                        textColor.x = settingsJson["textColor"][0];
                        textColor.y = settingsJson["textColor"][1];
                        textColor.z = settingsJson["textColor"][2];
                        textColor.w = settingsJson["textColor"][3];
                    } else {
                        Log("Invalid or missing textColor settings");
                    }

                    Log("Settings loaded successfully");
                } else {
                    Log("Invalid or missing settings object");
                }
            } else {
                Log("Failed to load settings: " + message);
            }
        } else {
            Log("Invalid or missing status in response");
        }
    } catch (const std::exception& e) {
        Log("Settings load failed with Exception: " + std::string(e.what()));
    }
}

std::string FetchDataFromAPI(const std::string& url) {
    try {
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) throw std::runtime_error("Failed to open URL");

        std::string response = ReadResponse(hConnect);
        CloseInternetHandles(nullptr, hConnect, hInternet);
        return response;
    } catch (const std::exception& e) {
        Log(e.what());
        return "";
    }
}
std::vector<Pointer> InitializePointers() {
    std::vector<Pointer> pointers;
    std::string url = "https://api.sylent-x.com/pointers.php?username=" + login + "&password=" + password;
    std::string jsonResponse = FetchDataFromAPI(url);
    LogDebug("Fetched pointers from API: " + jsonResponse);
    
    if (!jsonResponse.empty()) {
        try {
            auto json = nlohmann::json::parse(jsonResponse);
            if (!json.contains("memory_pointers")) {
                Log("Invalid JSON response: memory_pointers not found");
                return pointers;
            }

            for (const auto& item : json["memory_pointers"].items()) {
                Pointer pointer;
                pointer.name = item.key();
                pointer.address = std::stoul(item.value()["address"].get<std::string>(), nullptr, 16);

                std::string offsetsStr = item.value()["offsets"].get<std::string>();
                if (!offsetsStr.empty()) {
                    std::stringstream ss(offsetsStr);
                    std::string offset;
                    while (std::getline(ss, offset, ',')) {
                        pointer.offsets.push_back(std::stoul(offset, nullptr, 16));
                    }
                }

                std::stringstream addressHex;
                addressHex << std::hex << pointer.address;
                LogDebug("Got pointer: Name = " + pointer.name + ", Address = 0x" + addressHex.str() + ", Offsets = " + offsetsStr);
                pointers.push_back(pointer);
            }
            LogDebug("Pointers fetched and parsed successfully");
        } catch (const nlohmann::json::exception& e) {
            LogDebug("JSON parsing error: " + std::string(e.what()));
        } catch (const std::invalid_argument& e) {
            LogDebug("Invalid address or offset format");
        }
    } else {
        Log("Failed to fetch or parse pointers");
    }

    return pointers;
}

std::vector<Pointer> g_pointers;

void RegisterUser(const std::string& username, const std::string& email, const std::string& password) {
    try {
        HINTERNET hSession = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hSession);
        std::string path = "/user.php?action=register&username=" + username + "&email=" + email + "&password=" + password;
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hSession);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            MessageBox(NULL, "Registration successful. Please activate your account by clicking the link in the e-mail.", "Success", MB_ICONINFORMATION | MB_TOPMOST);
        } else {
            MessageBox(NULL, ("Registration failed: " + message).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
        }
    } catch (const std::exception& e) {
        MessageBox(NULL, e.what(), "Exception", MB_ICONERROR | MB_TOPMOST);
    }
}
std::vector<std::string> g_chatMessages;

void SendChatMessage(const std::string& message) {
    try {
        std::string path = "/shoutbox.php?action=add&username=" + login + "&password=" + password + "&message=" + message;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("Chat message sent successfully");

            // Process the messages array
            auto messages = jsonResponse["messages"];
            std::unordered_set<std::string> existingMessages(g_chatMessages.begin(), g_chatMessages.end());
            for (const auto& msg : messages) {
                std::string createdAt = msg["created_at"];
                std::string user = msg["username"];
                std::string msgText = msg["message"];
                std::string fullMessage = "[" + createdAt + "] " + user + ": " + msgText;

                // Only store new messages
                if (existingMessages.find(fullMessage) == existingMessages.end()) {
                    g_chatMessages.push_back(fullMessage);
                    logMessages.push_back(fullMessage); // Add to logMessages
                    existingMessages.insert(fullMessage); // Update the set with the new message
                }
            }
        } else {
            Log("Failed to send chat message.");
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
    }
}

// Check for new chat messages every 2 seconds and store them in g_chatMessages
void CheckChatMessages() {
    bool keepRunning = true;
    while (keepRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        try {
            std::string path = "/shoutbox.php?action=get&username=" + login + "&password=" + password;
            HINTERNET hInternet = OpenInternetConnection();
            HINTERNET hConnect = ConnectToAPI(hInternet);
            HINTERNET hRequest = SendHTTPRequest(hConnect, path);
            std::string response = ReadResponse(hRequest);
            CloseInternetHandles(hRequest, hConnect, hInternet);

            auto jsonResponse = nlohmann::json::parse(response);
            std::string status = jsonResponse["status"];
            if (status == "success") {

                // Process the messages array
                auto messages = jsonResponse["messages"];
                std::unordered_set<std::string> existingMessages(g_chatMessages.begin(), g_chatMessages.end());
                for (const auto& msg : messages) {
                    std::string createdAt = msg["created_at"];
                    std::string user = msg["username"];
                    std::string msgText = msg["message"];
                    std::string fullMessage = "[" + createdAt + "] " + user + ": " + msgText;

                    // Only store new messages
                    if (existingMessages.find(fullMessage) == existingMessages.end()) {
                        g_chatMessages.push_back(fullMessage);
                        logMessages.push_back(fullMessage); // Add to logMessages
                        existingMessages.insert(fullMessage); // Update the set with the new message
                    }
                }
            } else {
                LogDebug("Failed to fetch chat messages.");
            }
        } catch (const std::exception& e) {
            Log("Exception: " + std::string(e.what()));
        }
    }
}

// ADMIN FUNCTIONS
// only available if isAdmin is true

std::string GetAllUsersRawJson;

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

// function to fetch amount of Magnat currency for the user
void GetMagnatCurrency() {
    try {
        std::string path = "/magnat.php?action=getWallet&username=" + login + "&password=" + password;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            magnatCurrency = jsonResponse["wallet"]["amount"];
            LogDebug("Magnat currency fetched successfully: " + std::to_string(magnatCurrency));
        } else {
            LogDebug("Failed to fetch Magnat currency: " + message);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
    }
}