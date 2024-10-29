#include "ApiHandler.h"

bool Login(const std::string& login, const std::string& password) {
    try {
        std::string path = "/user.php?action=login&username=" + login + "&password=" + password;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        auto jsonResponse = nlohmann::json::parse(response);

        // Check if status and message are not null
        if (jsonResponse.contains("status") && !jsonResponse["status"].is_null() &&
            jsonResponse.contains("message") && !jsonResponse["message"].is_null()) {
            std::string status = jsonResponse["status"];
            std::string message = jsonResponse["message"];

            if (status == "success") {
                LogDebug("User " + login + " logged in successfully");

                sylentx_status = jsonResponse["system_status"];
                LogDebug("Sylen-X Status: " + sylentx_status);

                sylentx_status = sylentx_status;

                auto licensedFeatures = jsonResponse["licensed_features"];
                featureZoom = std::find(licensedFeatures.begin(), licensedFeatures.end(), "zoom") != licensedFeatures.end();
                featureGravity = std::find(licensedFeatures.begin(), licensedFeatures.end(), "gravity") != licensedFeatures.end();
                featureMoonjump = std::find(licensedFeatures.begin(), licensedFeatures.end(), "moonjump") != licensedFeatures.end();
                featureMoonwalk = std::find(licensedFeatures.begin(), licensedFeatures.end(), "moonwalk") != licensedFeatures.end();
                featureFakelag = std::find(licensedFeatures.begin(), licensedFeatures.end(), "fakelag") != licensedFeatures.end();
                featureFov = std::find(licensedFeatures.begin(), licensedFeatures.end(), "fov") != licensedFeatures.end();
                featureSpeedhack = std::find(licensedFeatures.begin(), licensedFeatures.end(), "speedhack") != licensedFeatures.end();
                featureFastfly = std::find(licensedFeatures.begin(), licensedFeatures.end(), "fastfly") != licensedFeatures.end();
                featureFreecam = std::find(licensedFeatures.begin(), licensedFeatures.end(), "freecam") != licensedFeatures.end();

                Log("Licensed features: " + std::string(featureZoom ? "Zoom" : "") + 
                    std::string(featureGravity ? ", Gravity" : "") + 
                    std::string(featureMoonjump ? ", Moonjump" : "") + 
                    std::string(featureMoonwalk ? ", Moonwalk" : "") + 
                    std::string(featureFov ? ", Field of View" : "") + 
                    std::string(featureSpeedhack ? ", Speedhack" : "") + 
                    std::string(featureFastfly ? ", Fastfly" : "") + 
                    std::string(featureFakelag ? ", fakelag" : "") + 
                    std::string(featureFreecam ? ", Freecam" : ""));
                    
                // Check if role is not null
                if (jsonResponse.contains("role") && !jsonResponse["role"].is_null()) {
                    std::string role = jsonResponse["role"];
                    isAdmin = (role == "admin");
                    Log("Role: " + role);
                } else {
                    isAdmin = false;
                    Log("Role: unknown");
                }

                // Check if runtime_end is not null
                if (jsonResponse.contains("runtime_end") && !jsonResponse["runtime_end"].is_null()) {
                    license_runtime_end = jsonResponse["runtime_end"];
                } else {
                    license_runtime_end = "";
                }

                // Convert the licensed_features array to a comma-separated string
                std::ostringstream oss;
                for (const auto& feature : jsonResponse["licensed_features"]) {
                    if (oss.tellp() > 0) oss << ", ";
                    oss << feature.get<std::string>();
                }
                license_features = oss.str();

                g_pointers = InitializePointers();
                GetMagnatCurrency();

                // Start the CheckChatMessages process in a new thread
                std::thread chatThread(CheckChatMessages);
                chatThread.detach(); // Detach the thread to run independently

                return true;
            } else {
                return false;
            }
        } else {
            Log("Invalid response: missing status or message");
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
                    setting_fontSize = settingsJson.value("fontSize", 1.0f);
                    setting_enableRainbow = settingsJson.value("enableRainbow", false);
                    setting_rainbowSpeed = settingsJson.value("rainbowSpeed", 0.1f);  
                    setting_log_debug = settingsJson.value("logDebug", false);
                    setting_excludeFromCapture = settingsJson.value("excludeFromCapture", false);
                    setting_regnumInstallPath = settingsJson.value("regnumInstallPath", "C:\\Games\\NGD Studios\\Champions of Regnum");
                    enableMusic = settingsJson.value("enableMusic", true);
                    enableSoundEffects = settingsJson.value("enableSoundEffects", true);
                    showLoadingScreen = settingsJson.value("showLoadingScreen", true);
                    ShowIntro = settingsJson.value("showIntro", true);
                    soundVolume = settingsJson.value("SoundVolume", 0.5f);

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
            // MessageBox(NULL, "Login successful after saving credentials - Please restart the application to apply your license", "Success", MB_ICONINFORMATION | MB_TOPMOST);
            LoadLoginCredentials(hInstanceGlobal);
            LoadSettings();
            InitializePointers();
        } else {
            MessageBox(NULL, "Login failed after saving credentials", "Error", MB_ICONERROR | MB_TOPMOST);
        }
    } else {
        MessageBox(NULL, "Failed to open config file for writing", "Error", MB_ICONERROR | MB_TOPMOST);
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

void RegisterUser(const std::string& username, const std::string& nickname, const std::string& email, const std::string& password) {
    try {
        HINTERNET hSession = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hSession);
        std::string path = "/user.php?action=register&username=" + username + + "&nickname=" + nickname + "&email=" + email + "&password=" + password;
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
            auto messages = jsonResponse["messages"];
            std::unordered_set<std::string> existingMessages(g_chatMessages.begin(), g_chatMessages.end());
            for (const auto& msg : messages) {
                std::string createdAt = msg["created_at"];
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

void SendFeedback(const std::string& type, const std::string& feedback, bool feedback_includeLogfile) {
    try {
        std::string logContent;
        if (feedback_includeLogfile) {
            for (const auto& logMessage : logMessages) {
                logContent += logMessage + "\n";
            }
        }

        nlohmann::json requestBody;
        requestBody["type"] = type;
        requestBody["username"] = login;
        requestBody["password"] = password;
        requestBody["feedback"] = feedback;
        if (!logContent.empty()) {
            requestBody["log"] = logContent;
        }

        std::string requestBodyStr = requestBody.dump();

        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/feedback.php", NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
        if (!hRequest) {
            throw std::runtime_error("Failed to open HTTP request");
        }

        const char* headers = "Content-Type: application/json";
        INTERNET_BUFFERSA buffers = {0};
        buffers.dwStructSize = sizeof(INTERNET_BUFFERSA);
        buffers.lpcszHeader = headers;
        buffers.dwHeadersLength = (DWORD)strlen(headers);
        buffers.dwBufferTotal = (DWORD)requestBodyStr.length();

        if (!HttpSendRequestEx(hRequest, &buffers, NULL, HSR_INITIATE, 0)) {
            throw std::runtime_error("Failed to send HTTP request");
        }

        DWORD bytesWritten;
        if (!InternetWriteFile(hRequest, requestBodyStr.c_str(), (DWORD)requestBodyStr.length(), &bytesWritten)) {
            throw std::runtime_error("Failed to write request body");
        }

        if (!HttpEndRequest(hRequest, NULL, 0, 0)) {
            throw std::runtime_error("Failed to end HTTP request");
        }

        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        // Check if the response is valid JSON
        if (response.empty() || response[0] != '{') {
            Log("Invalid response received: " + response);
            return;
        }

        auto jsonResponse = nlohmann::json::parse(response);
        std::string status = jsonResponse["status"];
        std::string message = jsonResponse["message"];

        if (status == "success") {
            LogDebug("Feedback sent successfully: " + message);
        } else {
            LogDebug("Failed to send feedback: " + message);
        }
    } catch (const std::exception& e) {
        Log("Exception: " + std::string(e.what()));
    }
}

void ActivateLicense(const std::string& licenseKey) {
    try {
        std::string path = "/license.php?action=activate&username=" + login + "&password=" + password + "&key=" + licenseKey;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
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
