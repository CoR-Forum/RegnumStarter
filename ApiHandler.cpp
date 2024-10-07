#include "Utils.h"
#include <stdexcept>
#include <sstream>

#pragma once //added for register in sylent-x.cpp maybe check it 

#define WM_CLOSE_REGISTRATION_WINDOW (WM_USER + 1)

extern HWND hwnd; // Declare the handle to the main window

// Declare the Pointer struct
struct Pointer {
    std::string name;
    unsigned long address;
    std::vector<unsigned long> offsets;
};

extern bool featureZoom;
extern bool featureGravity;

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
        std::string path = "/login.php?username=" + login + "&password=" + password;
        HINTERNET hInternet = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hInternet);
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hInternet);

        if (response.find("\"status\":\"success\"") != std::string::npos) {
            Log("User " + login + " logged in successfully");

            featureZoom = response.find("\"zoom\"") != std::string::npos;
            featureGravity = response.find("\"gravity\"") != std::string::npos;

            Log("Licensed features: " + std::string(featureZoom ? "Zoom" : "") + std::string(featureGravity ? ", Gravity" : ""));
            return true;
        } else {
            Log("Failed to log in: " + response);
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
    std::string settingsDir = std::string(appDataPath) + "\\Sylent-X";
    std::string settingsFilePath = settingsDir + "\\settings.txt";

    CreateDirectory(settingsDir.c_str(), NULL);

    std::ofstream file(settingsFilePath);
    if (file.is_open()) {
        file << "optionGravity=" << optionGravity << std::endl;
        file << "optionMoonjump=" << optionMoonjump << std::endl;
        file << "optionZoom=" << optionZoom << std::endl;
        file.close();
    } else {
        Log("Failed to open settings file for writing");
    }
}

void LoadSettings() {
    std::string settingsFilePath = std::string(appDataPath) + "\\Sylent-X\\settings.txt";

    std::ifstream file(settingsFilePath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("optionGravity=") != std::string::npos)
                optionGravity = (line.substr(line.find("=") + 1) == "1");
            if (line.find("optionMoonjump=") != std::string::npos)
                optionMoonjump = (line.substr(line.find("=") + 1) == "1");
            if (line.find("optionZoom=") != std::string::npos)
                optionZoom = (line.substr(line.find("=") + 1) == "1");
        }
        file.close();
    } else {
        Log("Settings file not found");
    }

    LoadLoginCredentials(hInstanceGlobal);
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

std::vector<Pointer> ParseJSONResponse(const std::string& jsonResponse) {
    std::vector<Pointer> pointers;
    size_t pos = 0, endPos;

    while ((pos = jsonResponse.find("{", pos)) != std::string::npos) {
        Pointer pointer;
        endPos = jsonResponse.find("}", pos);
        std::string object = jsonResponse.substr(pos, endPos - pos + 1);

        size_t namePos = object.find("\"name\":") + 8;
        size_t nameEnd = object.find("\"", namePos);
        pointer.name = object.substr(namePos, nameEnd - namePos);

        size_t addressPos = object.find("\"address\":") + 11;
        size_t addressEnd = object.find("\"", addressPos);
        std::string addressStr = object.substr(addressPos, addressEnd - addressPos);
        try {
            pointer.address = std::stoul(addressStr, nullptr, 16);
        } catch (const std::invalid_argument& e) {
            LogDebug("Invalid address: " + addressStr);
            continue;
        }

        size_t offsetsPos = object.find("\"offsets\":") + 11;
        size_t offsetsEnd = object.find("\"", offsetsPos);
        std::string offsetsStr = object.substr(offsetsPos, offsetsEnd - offsetsPos);

        if (offsetsStr.empty()) {
            LogDebug("No offsets for pointer: " + pointer.name);
        } else {
            size_t offsetPos = 0, offsetEnd;
            while ((offsetEnd = offsetsStr.find(",", offsetPos)) != std::string::npos) {
                std::string offsetStr = offsetsStr.substr(offsetPos, offsetEnd - offsetPos);
                try {
                    pointer.offsets.push_back(std::stoul(offsetStr, nullptr, 16));
                } catch (const std::invalid_argument& e) {
                    LogDebug("Invalid offset: " + offsetStr);
                    continue;
                }
                offsetPos = offsetEnd + 1;
            }
            try {
                pointer.offsets.push_back(std::stoul(offsetsStr.substr(offsetPos), nullptr, 16));
            } catch (const std::invalid_argument& e) {
                LogDebug("Invalid offset: " + offsetsStr.substr(offsetPos));
                continue;
            }
        }

        LogDebug("Fetched pointer: Name = " + pointer.name + ", Address = " + std::to_string(pointer.address));
        pointers.push_back(pointer);
        pos = endPos + 1;
    }

    return pointers;
}

std::vector<Pointer> pointers;

void InitializePointers() {
    std::string url = "https://cort.cor-forum.de/api/v1/sylentx/memory/pointers?key=aingu8gaiv0daitoj6eeweezeug7Ei";
    std::string jsonResponse = FetchDataFromAPI(url);
    if (!jsonResponse.empty()) {
        pointers = ParseJSONResponse(jsonResponse);
        Log("Pointers fetched and parsed successfully");
    } else {
        Log("Failed to fetch or parse pointers");
    }
}

void RegisterUser(const std::string& username, const std::string& email, const std::string& password) {
    try {
        HINTERNET hSession = OpenInternetConnection();
        HINTERNET hConnect = ConnectToAPI(hSession);
        std::string path = "/register.php?username=" + username + "&email=" + email + "&password=" + password;
        HINTERNET hRequest = SendHTTPRequest(hConnect, path);
        std::string response = ReadResponse(hRequest);
        CloseInternetHandles(hRequest, hConnect, hSession);

        size_t messagePos = response.find("\"message\":\"") + 11;
        size_t messageEnd = response.find("\"", messagePos);
        std::string message = response.substr(messagePos, messageEnd - messagePos);

        if (response.find("\"status\":\"success\"") != std::string::npos) {
            MessageBox(NULL, "Registration successful. Please activate your account by clicking the link in the e-mail.", "Success", MB_ICONINFORMATION | MB_TOPMOST);
            SendMessage(hRegistrationWindow, WM_CLOSE_REGISTRATION_WINDOW, 0, 0);
        } else {
            MessageBox(NULL, ("Registration failed: " + message).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
        }
    } catch (const std::exception& e) {
        MessageBox(NULL, e.what(), "Exception", MB_ICONERROR | MB_TOPMOST);
    }
}
