#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <sstream>
#include "Logger.cpp" // Include the Logger for logging

// Declare the Pointer struct
struct Pointer {
    std::string name;
    unsigned long address;
    std::vector<unsigned long> offsets;
};

extern void Log(const std::string& message);
extern void LogDebug(const std::string& message);

// load login and password from settings.txt
extern std::string login;
extern std::string password;

extern bool featureZoom;
extern bool featureGravity;

bool Login(const std::string& login, const std::string& password) {
    std::string path = "/public/login.php?username=" + login + "&password=" + password;

    HINTERNET hInternet = InternetOpen("Sylent-X", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        Log("Failed to open internet connection");
        return false;
    }

    HINTERNET hConnect = InternetConnect(hInternet, "localhost", 5500, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        Log("Failed to connect to API");
        InternetCloseHandle(hInternet);
        return false;
    }

    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", path.c_str(), NULL, NULL, acceptTypes, 0, 0);
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

    HINTERNET hConnect = InternetConnect(hSession, "api.example.com", INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        Log("Failed to connect to server");
        InternetCloseHandle(hSession);
        return;
    }

    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/register", NULL, NULL, NULL, 0, 0);
    if (!hRequest) {
        Log("Failed to open HTTP request");
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return;
    }

    std::string postData = "username=" + username + "&email=" + email + "&password=" + password;
    const char* headers = "Content-Type: application/x-www-form-urlencoded";

    if (!HttpSendRequest(hRequest, headers, strlen(headers), (LPVOID)postData.c_str(), postData.length())) {
        Log("Failed to send HTTP request");
    } else {
        Log("Registration successful");
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
}