#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <sstream>
#include "Logger.cpp" // Include the Logger for logging

#define WM_CLOSE_REGISTRATION_WINDOW (WM_USER + 1)
#define WM_OPEN_LOGIN_WINDOW (WM_USER + 2)

extern HWND hwnd; // Declare the handle to the main window

// Declare the Pointer struct
struct Pointer {
    std::string name;
    unsigned long address;
    std::vector<unsigned long> offsets;
};

extern bool featureZoom;
extern bool featureGravity;

bool Login(const std::string& login, const std::string& password) {
    std::string path = "/login.php?username=" + login + "&password=" + password;

    HINTERNET hInternet = InternetOpen("Sylent-X", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        Log("Failed to open internet connection");
        return false;
    }

    HINTERNET hConnect = InternetConnect(hInternet, "api.sylent-x.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        Log("Failed to connect to API");
        InternetCloseHandle(hInternet);
        return false;
    }

    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", path.c_str(), NULL, NULL, acceptTypes, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        Log("Failed to open HTTP request");
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    if (!HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
        Log("Failed to send HTTP request");
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;
    std::string response;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
        response.append(buffer, bytesRead);
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    // Parse the response manually
    if (response.find("\"status\":\"success\"") != std::string::npos) {
        Log("User " + login + " logged in successfully: " + response);

        // Reset features
        featureZoom = false;
        featureGravity = false;

        // Check for licensed features in the response
        if (response.find("\"licensed_features\":[") != std::string::npos) {
            size_t startPos = response.find("\"licensed_features\":[") + 21;
            size_t endPos = response.find("]", startPos);
            std::string featuresStr = response.substr(startPos, endPos - startPos);

            if (featuresStr.find("\"zoom\"") != std::string::npos) {
                featureZoom = true;
            }
            if (featuresStr.find("\"gravity\"") != std::string::npos) {
                featureGravity = true;
            }
        }

        Log("Licensed features: " + std::string(featureZoom ? "Zoom" : "") + std::string(featureGravity ? ", Gravity" : ""));

        return true;
    } else {
        Log("Failed to log in: " + response);
        return false;
    }
}

void Logout() {
    // Clear login credentials
    login.clear();
    password.clear();

    // Update the config file to remove login and password
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

    Log("Login credentials removed from config file");
    
    // Quit
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
        Log("Login credentials loaded successfully");
    } else {
        Log("Failed to open config file for reading");
    }

    if (!loginFound || !passwordFound) {
        Log("Login or password not found in config file. Opening login window.");
        OpenLoginWindow();
    }
}

void SaveLoginCredentials(const std::string& login, const std::string& password) {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\config.txt";

    std::ofstream file(configFilePath);
    if (file.is_open()) {
        file << "login=" << login << std::endl;
        file << "password=" << password << std::endl;
        file.close();
        Log("Login credentials saved successfully");

        // Attempt to login again
        if (Login(login, password)) {
            Log("Login successful after saving credentials - Please restart the application to apply your license");
            MessageBox(NULL, "Login successful! Please restart the application to apply your license.", "Success", MB_ICONINFORMATION);
            // quit the application
            PostQuitMessage(0);
        } else {
            Log("Login failed after saving credentials");
            // open login window again
            OpenLoginWindow();
        }
    } else {
        Log("Failed to open config file for writing");
    }
}

void SaveSettings() {
    Log("Saving settings to file");

    // Construct the settings file path
    std::string settingsDir = std::string(appDataPath) + "\\Sylent-X";
    std::string settingsFilePath = settingsDir + "\\settings.txt";

    // Create the directory if it doesn't exist
    CreateDirectory(settingsDir.c_str(), NULL);

    // Open the file and write the settings
    std::ofstream file(settingsFilePath);
    if (file.is_open()) {
        file << "optionGravity=" << optionGravity << std::endl;
        file << "optionMoonjump=" << optionMoonjump << std::endl;
        file << "optionZoom=" << optionZoom << std::endl;
        file.close();
        Log("Settings saved successfully");
    } else {
        Log("Failed to open settings file for writing");
    }
}

void LoadSettings() {
    LogDebug("Loading settings from file");

    // Construct the settings file path
    std::string settingsFilePath = std::string(appDataPath) + "\\Sylent-X\\settings.txt";

    // Open the file and read the settings
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
        Log("Settings loaded successfully");
    } else {
        LogDebug("Settings file not found");
    }

    // Load login credentials
    LoadLoginCredentials(hInstanceGlobal);
}

std::string FetchDataFromAPI(const std::string& url) {
    HINTERNET hInternet = InternetOpen("Sylent-X", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        Log("Failed to open internet connection");
        return "";
    }

    HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        Log("Failed to open URL");
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[4096];
    DWORD bytesRead;
    std::string response;

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
        response.append(buffer, bytesRead);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
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
        // LogDebug("Offsets string: " + offsetsStr); // Add this line to log the offsets string

        if (offsetsStr.empty()) {
            LogDebug("No offsets for pointer: " + pointer.name);
        } else {
            size_t offsetPos = 0, offsetEnd;
            while ((offsetEnd = offsetsStr.find(",", offsetPos)) != std::string::npos) {
                std::string offsetStr = offsetsStr.substr(offsetPos, offsetEnd - offsetPos);
                try {
                    pointer.offsets.push_back(std::stoul(offsetStr, nullptr, 16));
                    // LogDebug("Offset: " + offsetStr); // Add this line to log the offset
                } catch (const std::invalid_argument& e) {
                    LogDebug("Invalid offset: " + offsetStr);
                    continue;
                }
                offsetPos = offsetEnd + 1;
            }
            try {
                pointer.offsets.push_back(std::stoul(offsetsStr.substr(offsetPos), nullptr, 16));
                // LogDebug("Last offset: " + offsetsStr.substr(offsetPos)); // Add this line to log the last offset
            } catch (const std::invalid_argument& e) {
                LogDebug("Invalid offset: " + offsetsStr.substr(offsetPos));
                continue;
            }
        }

        // Log the fetched data
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
    HINTERNET hSession = InternetOpen("RegistrationAgent", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession) {
        Log("Failed to open internet session");
        return;
    }

    HINTERNET hConnect = InternetConnect(hSession, "api.sylent-x.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        Log("Failed to connect to server");
        InternetCloseHandle(hSession);
        return;
    }

    std::string path = "/register.php?username=" + username + "&email=" + email + "&password=" + password;
    HINTERNET hRequest = HttpOpenRequest(hConnect, "GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        Log("Failed to open HTTP request");
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return;
    }

    if (!HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
        Log("Failed to send HTTP request");
    } else {
        // Log the response
        char buffer[4096];
        DWORD bytesRead;
        std::string response;

        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
            response.append(buffer, bytesRead);
        }

        Log("Registration response: " + response);

        // Parse the response and show the message
        size_t messagePos = response.find("\"message\":\"") + 11;
        size_t messageEnd = response.find("\"", messagePos);
        std::string message = response.substr(messagePos, messageEnd - messagePos);

        if (response.find("\"status\":\"success\"") != std::string::npos) {
            Log("User registered successfully: " + message);
            // Send message to close the registration window
            SendMessage(hRegistrationWindow, WM_CLOSE_REGISTRATION_WINDOW, 0, 0);
        } else {
            Log("Registration failed: " + message);
        }
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
}
