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
    if (!hConnect) throw std::runtime_error("Failed to connect to API on localhost:3000");
    return hConnect;
}

HINTERNET SendHTTPRequest(HINTERNET hConnect, const std::string& path) {
    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", path.c_str(), NULL, NULL, acceptTypes, 0, 0); // Removed INTERNET_FLAG_SECURE
    if (!hRequest || !HttpSendRequest(hRequest, NULL, 0, NULL, 0)) {
        throw std::runtime_error("Failed to open or send HTTP request");
    }
    return hRequest;
}

HINTERNET SendHTTPPostRequest(HINTERNET hConnect, const std::string& path, const std::string& requestBody) {
    Log("Sending POST request to " + path + " with body: " + requestBody);
    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", path.c_str(), NULL, NULL, acceptTypes, 0, 0); // Removed INTERNET_FLAG_SECURE
    if (!hRequest) {
        std::cerr << "HttpOpenRequest failed with error: " << GetLastError() << std::endl;
        throw std::runtime_error("Failed to open HTTP request");
    }

    const char* headers = "Content-Type: application/json\r\n";
    if (!HttpSendRequest(hRequest, headers, (DWORD)strlen(headers), (LPVOID)requestBody.c_str(), (DWORD)requestBody.length())) {
        std::cerr << "HttpSendRequest failed with error: " << GetLastError() << std::endl;
        InternetCloseHandle(hRequest);
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