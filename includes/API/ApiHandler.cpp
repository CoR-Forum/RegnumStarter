#include "ApiHandler.h"

void SaveLoginSettings(const std::string& username, bool saveUsername) {
    std::string configFilePath = std::string(getenv("APPDATA")) + "\\Sylent-X\\login-settings.json";
    nlohmann::json loginSettingsJson;

    if (saveUsername) {
        loginSettingsJson["username"] = username;
    } else {
        loginSettingsJson["username"] = "";
    }
    loginSettingsJson["saveUsername"] = saveUsername;

    std::ofstream outFile(configFilePath);
    outFile << loginSettingsJson.dump(4);
    outFile.close();
}

void LoadLoginSettings() {
    std::string configFilePath = std::string(getenv("APPDATA")) + "\\Sylent-X\\login-settings.json";
    std::ifstream inFile(configFilePath);
    if (inFile.is_open()) {
        nlohmann::json loginSettingsJson;
        inFile >> loginSettingsJson;
        inFile.close();

        login = loginSettingsJson.value("username", "");
        saveUsername = loginSettingsJson.value("saveUsername", false);
    }
}

std::pair<bool, std::string> Login(const std::string& login, const std::string& password) {
    try {
        std::string path = "/v1/login";
        nlohmann::json jsonPayload = {
            {"username", login},
            {"password", password}
        };
        std::string payload = jsonPayload.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
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
                    return {false, "Invalid response: missing token"};
                }

                if (jsonResponse.contains("user") && jsonResponse["user"].is_object()) {
                    auto user = jsonResponse["user"];
                    LogDebug("User object found");

                    std::string userId = user.value("id", "");
                    std::string username = user.value("username", "");
                    std::string nickname = user.value("nickname", "");
                    std::string settings = user.value("settings", "");

                    LogDebug("Loading settings: User ID: " + userId + ", Username: " + username + ", Nickname: " + nickname + ", Settings: " + settings + ", Features: " + user["features"].dump());

                    // Deserialize settings JSON string
                    auto settingsJson = nlohmann::json::parse(settings);
                    setting_log_debug = settingsJson.value("logDebug", false);
                    textColor = ImVec4(
                        settingsJson["textColor"][0],
                        settingsJson["textColor"][1],
                        settingsJson["textColor"][2],
                        settingsJson["textColor"][3]
                    );
                    setting_excludeFromCapture = settingsJson.value("excludeFromCapture", false);
                    setting_regnumInstallPath = settingsJson.value("regnumInstallPath", "");
                    enableMusic = settingsJson.value("enableMusic", true);
                    enableSoundEffects = settingsJson.value("enableSoundEffects", true);
                    showLoadingScreen = settingsJson.value("showLoadingScreen", true);
                    showIntro = settingsJson.value("showIntro", true);
                    soundVolume = settingsJson.value("SoundVolume", 0.5f);

                    // Save username and saveUsername flag to file
                    SaveLoginSettings(username, saveUsername);

                    if (user.contains("features") && user["features"].is_array()) {
                        auto features = user["features"];

                        for (const auto& feature : features) {
                            if (feature.contains("pointer") && feature["pointer"].is_object()) {
                                auto pointerObj = feature["pointer"];
                                std::string featureName = feature.value("name", "");
                                std::string address = pointerObj.value("address", "");
                                auto offsets = pointerObj.value("offsets", std::vector<std::string>());

                                //LogDebug("Pointer: " + featureName + ", Address: " + address + ", Offsets: " + nlohmann::json(offsets).dump());

                                Pointer pointer;
                                pointer.name = featureName;
                                pointer.address = std::stoul(address, nullptr, 16);

                                for (const auto& offset : offsets) {
                                    pointer.offsets.push_back(std::stoul(offset, nullptr, 16));
                                }

                                std::stringstream addressHex;
                                addressHex << std::hex << pointer.address;
                                //LogDebug("Got pointer: Name = " + pointer.name + ", Address = 0x" + addressHex.str() + ", Offsets = " + nlohmann::json(offsets).dump());
                                g_pointers.push_back(pointer);

                                // Update feature flags based on feature name
                                if (featureName == "zoom") {
                                    featureZoom = true;
                                } else if (featureName == "gravity") {
                                    featureGravity = true;
                                } else if (featureName == "moonjump") {
                                    featureMoonjump = true;
                                } else if (featureName == "moonwalk") {
                                    featureMoonwalk = true;
                                } else if (featureName == "fov") {
                                    featureFov = true;
                                } else if (featureName == "speedhack") {
                                    featureSpeedhack = true;
                                } else if (featureName == "freecam") {
                                    featureFreecam = true;
                                } else if (featureName == "fastfly") {
                                    featureFastfly = true;
                                } else if (featureName == "fakelag") {
                                    featureFakelag = true;
                                } else if (featureName == "character") {
                                    featureCharacter = true;
                                }
                            }
                        }
                        LogDebug("Pointers fetched and parsed successfully");
                    }

                    // Initialize other necessary variables and features here
                    SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
                    std::thread chatThread(CheckChatMessages);
                    chatThread.detach();

                    return {true, "Login successful"};
                } else {
                    Log("Invalid response: missing user object");
                    return {false, "Invalid response: missing user object"};
                }
            } else {
                Log("Login failed: " + message);
                return {false, message};
            }
        } else if (jsonResponse.contains("status") && jsonResponse["status"] == "error" &&
                   jsonResponse.contains("message") && jsonResponse["message"].is_string()) {
            std::string message = jsonResponse["message"];
            Log("Login failed: " + message);
            return {false, message};
        } else {
            Log("Invalid response: missing or invalid status/message");
            return {false, "Invalid response: missing or invalid status/message"};
        }
    } catch (const std::exception& e) {
        Log(e.what());
        return {false, "LOGIN EXCEPTION: " + std::string(e.what())};
    }
}

void SaveSettings() {
    try {
        nlohmann::json settingsJson;
        settingsJson["logDebug"] = setting_log_debug;
        settingsJson["textColor"] = { textColor.x, textColor.y, textColor.z, textColor.w };
        settingsJson["excludeFromCapture"] = setting_excludeFromCapture;
        settingsJson["regnumInstallPath"] = setting_regnumInstallPath;
        settingsJson["enableMusic"] = enableMusic;
        settingsJson["enableSoundEffects"] = enableSoundEffects;
        settingsJson["showLoadingScreen"] = showLoadingScreen;
        settingsJson["showIntro"] = showIntro;
        settingsJson["SoundVolume"] = soundVolume;

        // Serialize settingsJson to a string
        std::string settingsString = settingsJson.dump();

        nlohmann::json payloadJson;
        payloadJson["settings"] = settingsString; // Store the serialized JSON string

        std::string payload = payloadJson.dump();
        Log("Payload being sent: " + payload); // Log the payload
 
        std::string path = "/v1/save-settings";

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPPutRequest(hConnect, path, payload);
        std::string response = ReadResponse(hRequest);
        Log("Response received: " + response); // Log the response

        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse.value("status", "");
        std::string message = jsonResponse.value("message", "");

        if (status == "success") {
            Log("Settings saved successfully: " + message + ", Payload: " + payload);
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
        std::string path = "/v1/logout";

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, "");
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
        HINTERNET hConnect = ConnectToAPI(hInternet);
        std::string path = "/v1/register";

        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload);
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
        HINTERNET hConnect = ConnectToAPI(hInternet);
        std::string path = "/v1/reset-password";

        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload);
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
        std::string path = "/v1/reset-password/" + token;
        nlohmann::json jsonPayload = {
            {"password", password}
        };
        std::string payload = jsonPayload.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload);
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
        std::string path = "/v1/chat/send";
        nlohmann::json jsonPayload = {
            {"message", message}
        };
        std::string payload = jsonPayload.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPPostRequest(hConnect, path, payload);
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
            std::string path = "/v1/chat/receive";

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
                    std::string createdAt = msg["timestamp"];
                    std::string user = msg.contains("nickname") ? msg["nickname"] : "Unknown";
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
        std::string path = "/v1/license/activate";
        nlohmann::json jsonPayload = {
            {"licenseKey", licenseKey}
        };
        std::string payload = jsonPayload.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPPutRequest(hConnect, path, payload);
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
    std::string folderPath = std::string(appDataPath) + "\\Sylent-X";


    std::filesystem::create_directories(folderPath);

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