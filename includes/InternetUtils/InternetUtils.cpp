// InternetUtils.cpp
#include "InternetUtils.h"
#include <stdexcept>
#include <wininet.h>

void CloseInternetHandles(HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet) {
    if (hRequest) InternetCloseHandle(hRequest);
    if (hConnect) InternetCloseHandle(hConnect);
    if (hInternet) InternetCloseHandle(hInternet);
}

bool LogAndCloseHandles(const std::string& message, HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet) {
    Log(message);
    CloseInternetHandles(hRequest, hConnect, hInternet);
    return true;
}

HINTERNET OpenInternetConnection() {
    HINTERNET hInternet = InternetOpen("Sylent-X", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) throw std::runtime_error("Failed to open internet connection");
    return hInternet;
}

HINTERNET ConnectToAPI(HINTERNET hInternet) {
    HINTERNET hConnect = InternetConnect(hInternet, "api.sylent-x.com", INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) throw std::runtime_error("Failed to connect to API");
    return hConnect;
}

HINTERNET ConnectToAPIv2(HINTERNET hInternet) {
    HINTERNET hConnect = InternetConnect(hInternet, "localhost", 3000, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) throw std::runtime_error("Failed to connect to API on 5.161.184.121:3000");
    return hConnect;
}

HINTERNET SendHTTPRequest(HINTERNET hConnect, const std::string& path) {
    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "GET", path.c_str(), NULL, NULL, acceptTypes, 0, 0);

    std::string headers = "Content-Type: application/json";
    if (!session_id.empty()) {
        headers += "\r\nAuthorization:" + session_id;
    }

    BOOL result = HttpSendRequest(hRequest, headers.c_str(), headers.length(), NULL, 0);
    if (!result) {
        throw std::runtime_error("Failed to send HTTP request");
    }

    return hRequest;
}

HINTERNET SendHTTPPostRequest(HINTERNET hConnect, const std::string& path, const std::string& payload) {
    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", path.c_str(), NULL, NULL, acceptTypes, 0, 0);

    std::string headers = "Content-Type: application/json\r\n";
    if (!session_id.empty()) {
        headers += "Authorization: " + session_id + "\r\n";
    }

    BOOL result = HttpSendRequest(hRequest, headers.c_str(), headers.length(), (LPVOID)payload.c_str(), payload.length());
    if (!result) {
        throw std::runtime_error("Failed to send HTTP request");
    }

    return hRequest;
}

HINTERNET SendHTTPPutRequest(HINTERNET hConnect, const std::string& path, const std::string& payload) {
    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "PUT", path.c_str(), NULL, NULL, acceptTypes, 0, 0);

    std::string headers = "Content-Type: application/json\r\n";
    if (!session_id.empty()) {
        headers += "Authorization: Bearer " + session_id + "\r\n";
    }

    BOOL result = HttpSendRequest(hRequest, headers.c_str(), headers.length(), (LPVOID)payload.c_str(), payload.length());
    if (!result) {
        throw std::runtime_error("Failed to send HTTP request");
    }

    return hRequest;
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

std::string ReadResponse(HINTERNET hRequest) {
    std::string response;
    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        response.append(buffer, bytesRead);
    }
    return response;
}