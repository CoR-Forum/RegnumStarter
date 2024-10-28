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