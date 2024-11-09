#include "ApiHandler.h"

std::string session_id;

bool Login(const std::string& login, const std::string& password) {
    try {
        std::string path = "/api/v2/login";
        nlohmann::json jsonPayload = {
            {"username", login},
            {"password", password}
        };
        std::string payload = jsonPayload.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload);
        std::string response = ReadResponse(hRequest);

        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);

        if (jsonResponse.contains("status") && jsonResponse["status"] == "success" &&
            jsonResponse.contains("message") && jsonResponse["message"].is_string()) {
            std::string message = jsonResponse["message"];

            if (message == "Login successful") {
                LogDebug("User " + login + " logged in successfully");

                if (jsonResponse.contains("token") && jsonResponse["token"].is_string()) {
                    session_id = jsonResponse["token"];
                    LogDebug("Session ID (JWT): " + session_id);
                } else {
                    Log("Invalid response: missing token");
                    return false;
                }

                if (jsonResponse.contains("user") && jsonResponse["user"].is_object()) {
                    auto user = jsonResponse["user"];

                    std::string userId = user.value("id", "");
                    std::string username = user.value("username", "");
                    std::string nickname = user.value("nickname", "");
                    std::string settings = user.value("settings", "");

                    LogDebug("User ID: " + userId + ", Username: " + username + ", Nickname: " + nickname);

                    // Deserialize settings JSON string
                    auto settingsJson = nlohmann::json::parse(settings);
                    setting_log_debug = settingsJson.value("logDebug", false);
                    textColor = ImVec4(
                        settingsJson["textColor"][0],
                        settingsJson["textColor"][1],
                        settingsJson["textColor"][2],
                        settingsJson["textColor"][3]
                    );
                    setting_fontSize = settingsJson.value("fontSize", 14.0f);
                    setting_enableRainbow = settingsJson.value("enableRainbow", false);
                    setting_rainbowSpeed = settingsJson.value("rainbowSpeed", 0.1f);
                    setting_excludeFromCapture = settingsJson.value("excludeFromCapture", false);
                    setting_regnumInstallPath = settingsJson.value("regnumInstallPath", "");
                    enableMusic = settingsJson.value("enableMusic", true);
                    enableSoundEffects = settingsJson.value("enableSoundEffects", true);
                    showLoadingScreen = settingsJson.value("showLoadingScreen", true);
                    ShowIntro = settingsJson.value("showIntro", true);
                    soundVolume = settingsJson.value("SoundVolume", 0.5f);

                    // Log the loaded settings
                    LogDebug("Loaded settings: logDebug=" + std::to_string(setting_log_debug) +
                             ", textColor=(" + std::to_string(textColor.x) + ", " + std::to_string(textColor.y) + ", " + std::to_string(textColor.z) + ", " + std::to_string(textColor.w) + ")" +
                             ", fontSize=" + std::to_string(setting_fontSize) +
                             ", enableRainbow=" + std::to_string(setting_enableRainbow) +
                             ", rainbowSpeed=" + std::to_string(setting_rainbowSpeed) +
                             ", excludeFromCapture=" + std::to_string(setting_excludeFromCapture) +
                             ", regnumInstallPath=" + setting_regnumInstallPath +
                             ", enableMusic=" + std::to_string(enableMusic) +
                             ", enableSoundEffects=" + std::to_string(enableSoundEffects) +
                             ", showLoadingScreen=" + std::to_string(showLoadingScreen) +
                             ", showIntro=" + std::to_string(ShowIntro) +
                             ", soundVolume=" + std::to_string(soundVolume));

                    if (user.contains("features") && user["features"].is_array()) {
                        auto features = user["features"];

                        for (const auto& feature : features) {
                            if (feature.contains("pointer") && feature["pointer"].is_object()) {
                                auto pointerObj = feature["pointer"];
                                std::string featureName = feature.value("name", "");
                                std::string address = pointerObj.value("address", "");
                                std::string offsets = pointerObj.value("offsets", "");

                                LogDebug("Pointer: " + featureName + ", Address: " + address + ", Offsets: " + offsets);

                                Pointer pointer;
                                pointer.name = featureName;
                                pointer.address = std::stoul(address, nullptr, 16);

                                if (!offsets.empty()) {
                                    std::stringstream ss(offsets);
                                    std::string offset;
                                    while (std::getline(ss, offset, ',')) {
                                        pointer.offsets.push_back(std::stoul(offset, nullptr, 16));
                                    }
                                }

                                std::stringstream addressHex;
                                addressHex << std::hex << pointer.address;
                                LogDebug("Got pointer: Name = " + pointer.name + ", Address = 0x" + addressHex.str() + ", Offsets = " + offsets);
                                g_pointers.push_back(pointer);
                            }
                        }
                        LogDebug("Pointers fetched and parsed successfully");
                    }

                    // Initialize other necessary variables and features here

                    std::thread chatThread(CheckChatMessages);
                    chatThread.detach();

                    return true;
                } else {
                    Log("Invalid response: missing user object");
                    return false;
                }
            } else {
                Log("Login failed: " + message);
                MessageBox(NULL, message.c_str(), "Login failed", MB_ICONERROR | MB_TOPMOST);
                return false;
            }
        } else if (jsonResponse.contains("status") && jsonResponse["status"] == "error" &&
                   jsonResponse.contains("message") && jsonResponse["message"].is_string()) {
            std::string message = jsonResponse["message"];
            Log("Login failed: " + message);
            MessageBox(NULL, message.c_str(), "Login failed", MB_ICONERROR | MB_TOPMOST);
            return false;
        } else {
            Log("Invalid response: missing or invalid status/message");
            return false;
        }
    } catch (const std::exception& e) {
        Log(e.what());
        return false;
    }
}

void SaveSettings() {
    try {
        nlohmann::json settingsJson;
        settingsJson["logDebug"] = setting_log_debug;
        settingsJson["textColor"] = { textColor.x, textColor.y, textColor.z, textColor.w };
        settingsJson["fontSize"] = setting_fontSize;
        settingsJson["enableRainbow"] = setting_enableRainbow;
        settingsJson["rainbowSpeed"] = setting_rainbowSpeed;  
        settingsJson["excludeFromCapture"] = setting_excludeFromCapture;
        settingsJson["regnumInstallPath"] = setting_regnumInstallPath;
        settingsJson["enableMusic"] = enableMusic;
        settingsJson["enableSoundEffects"] = enableSoundEffects;
        settingsJson["showLoadingScreen"] = showLoadingScreen;
        settingsJson["showIntro"] = ShowIntro;
        settingsJson["SoundVolume"] = soundVolume;

        // Serialize settingsJson to a string
        std::string settingsString = settingsJson.dump();

        nlohmann::json payloadJson;
        payloadJson["settings"] = settingsString; // Store the serialized JSON string

        std::string payload = payloadJson.dump();
        Log("Payload being sent: " + payload); // Log the payload

        std::string path = "/api/v2/save-settings";

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        HINTERNET hRequest = SendHTTPPutRequest(hConnect, path, payload, session_id); // Pass session_id as a header
        std::string response = ReadResponse(hRequest);
        Log("Response received: " + response); // Log the response

        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse.value("status", "");
        std::string message = jsonResponse.value("message", "");

        if (status == "success") {
            Log("Settings saved successfully: " + message);
        } else {
            Log("Failed to save settings: " + message);
        }
    } catch (const std::exception& e) {
        Log("Failed to save settings with exception: " + std::string(e.what()));
    }
}

void Logout() {
    SaveSettings(); // Save settings before logging out
    try {
        std::string path = "/api/v2/logout";

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, "", session_id); // Pass session_id as a header
        std::string response = ReadResponse(hRequest);
        Log("Response received: " + response); // Log the response

        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse.value("status", "");
        std::string message = jsonResponse.value("message", "");

        if (status == "success") {
            Log("Logged out successfully: " + message);
        } else {
            Log("Failed to log out: " + message);
        }
    } catch (const std::exception& e) {
        Log("Failed to log out with exception: " + std::string(e.what()));
    }

    session_id.clear();
    PostQuitMessage(0);
}

void RegisterUser(const std::string& username, const std::string& nickname, const std::string& email, const std::string& password) {
    try {
        nlohmann::json jsonPayload = {
            {"username", username},
            {"nickname", nickname},
            {"email", email},
            {"password", password}
        };
        std::string payload = jsonPayload.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        std::string path = "/api/v2/register";
        std::string headers = "Content-Type: application/json";

        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload, headers);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

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

bool ResetPasswordRequest(const std::string& email) {
    try {
        nlohmann::json jsonPayload = {
            {"email", email}
        };
        std::string payload = jsonPayload.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        std::string path = "/api/v2/reset-password";
        std::string headers = "Content-Type: application/json";

        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload, headers);
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
        std::string path = "/api/v2/reset-password/" + token;
        nlohmann::json jsonPayload = {
            {"password", password}
        };
        std::string payload = jsonPayload.dump();
        std::string headers = "Content-Type: application/json";

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload, headers);
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

void SendChatMessage(const std::string& message) {
    try {
        std::string path = "/api/v2/chat/send";
        nlohmann::json jsonPayload = {
            {"message", message}
        };
        std::string payload = jsonPayload.dump();
        std::string headers = "Content-Type: application/json";
        if (!session_id.empty()) {
            headers += "\r\nCookie: connect.sid=" + session_id;
        }

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload, headers);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            auto messages = jsonResponse["messages"];
            std::unordered_set<std::string> existingMessages(g_chatMessages.begin(), g_chatMessages.end());
            for (const auto& msg : messages) {
                std::string createdAt = msg["timestamp"];
                std::string user = msg["nickname"];
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
            std::string path = "/api/v2/chat/receive";
            std::string headers = "Content-Type: application/json";
            if (!session_id.empty()) {
                headers += "\r\nCookie: connect.sid=" + session_id;
            }

            HINTERNET hInternet = OpenInternetConnection();
            HINTERNET hConnect = ConnectToAPIv2(hInternet);
            HINTERNET hRequest = SendHTTPRequest(hConnect, path, headers);
            std::string response = ReadResponse(hRequest);
            CloseInternetHandles(hRequest, hConnect, hInternet);

            auto jsonResponse = nlohmann::json::parse(response);
            std::string status = jsonResponse["status"];
            if (status == "success") {

                // Process the messages array
                auto messages = jsonResponse["messages"];
                std::unordered_set<std::string> existingMessages(g_chatMessages.begin(), g_chatMessages.end());
                for (const auto& msg : messages) {
                    std::string createdAt = msg["timestamp"];
                    std::string user = msg["nickname"];
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

void ActivateLicense(const std::string& licenseKey) {
    try {
        std::string path = "/api/v2/license/activate";
        nlohmann::json jsonPayload = {
            {"licenseKey", licenseKey}
        };
        std::string payload = jsonPayload.dump();
        std::string headers = "Content-Type: application/json";
        if (!session_id.empty()) {
            headers += "\r\nCookie: connect.sid=" + session_id;
        }

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPIv2(hInternet);
        HINTERNET hRequest = SendHTTPPutRequest(hConnect, path, payload, headers);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("License activated successfully: " + message);
            MessageBox(NULL, "Please restart Sylent-X to complete the activation.", "Success", MB_ICONINFORMATION | MB_TOPMOST);
        } else {
            LogDebug("Failed to activate license: " + message);
            MessageBox(NULL, message.c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
    }
}

// Function to generate MD5 hash of a given string using header-only MD5 library
std::string GenerateMD5(const std::string& input) {
    return md5(input);
}

// Function to save a Regnum account to regnum-accounts.json appdata file with ID, username, password, server, and referrer
void SaveRegnumAccount(const std::string& username, const std::string& password, const std::string& server, const std::string& referrer, int id = -1) {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\regnum-accounts.json";
    std::ifstream file(configFilePath);
    nlohmann::json accountsJson;

    if (file.is_open()) {
        file >> accountsJson;
        file.close();
    }

    bool accountUpdated = false;

    std::string hashedPassword = GenerateMD5(password);

    if (id != -1) {
        // Try to find and update the existing account with the given ID
        for (auto& account : accountsJson) {
            if (account["id"].get<int>() == id) {
                account["username"] = username;
                account["password"] = hashedPassword;
                account["server"] = server;
                account["referrer"] = referrer;
                accountUpdated = true;
                break;
            }
        }
    }

    if (!accountUpdated) {
        // Generate a new ID for the account
        int newId = 1;
        if (!accountsJson.empty()) {
            newId = accountsJson.back()["id"].get<int>() + 1;
        }

        nlohmann::json newAccount;
        newAccount["id"] = newId;
        newAccount["username"] = username;
        newAccount["password"] = hashedPassword;
        newAccount["server"] = server;
        newAccount["referrer"] = referrer;
        accountsJson.push_back(newAccount);
    }

    std::ofstream outFile(configFilePath);
    outFile << accountsJson.dump(4);
    outFile.close();

    LoadRegnumAccounts();
}

// Function to load all Regnum accounts from regnum-accounts.json appdata file
void LoadRegnumAccounts() {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\regnum-accounts.json";
    std::ifstream file(configFilePath);
    nlohmann::json accountsJson;

    if (file.is_open()) {
        file >> accountsJson;
        file.close();
    }

    regnumAccounts.clear();
    for (const auto& account : accountsJson) {
        RegnumAccount regnumAccount;
        regnumAccount.id = account["id"];
        regnumAccount.username = account["username"];
        regnumAccount.password = account["password"];
        regnumAccount.server = account["server"];
        regnumAccount.referrer = account["referrer"];
        regnumAccounts.push_back(regnumAccount);
    }
    Log("Loaded " + std::to_string(regnumAccounts.size()) + " Regnum accounts");
}

// delete regnum account by id
void DeleteRegnumAccount(int id) {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\regnum-accounts.json";
    std::ifstream file(configFilePath);
    nlohmann::json accountsJson;

    if (file.is_open()) {
        file >> accountsJson;
        file.close();
    }

    for (auto it = accountsJson.begin(); it != accountsJson.end(); ++it) {
        if ((*it)["id"].get<int>() == id) {
            accountsJson.erase(it);
            break;
        }
    }

    std::ofstream outFile(configFilePath);
    outFile << accountsJson.dump(4);
    outFile.close();

    LoadRegnumAccounts();
}