#pragma once

#include <windows.h>
#include <wininet.h>
#include <string>

void CloseInternetHandles(HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet);
bool LogAndCloseHandles(const std::string& message, HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet);
HINTERNET OpenInternetConnection();
HINTERNET ConnectToAPI(HINTERNET hInternet);
HINTERNET SendHTTPRequest(HINTERNET hConnect, const std::string& path);
HINTERNET SendHTTPPostRequest(HINTERNET hConnect, const std::string& path, const std::string& requestBody);
HINTERNET SendHTTPPutRequest(HINTERNET hConnect, const std::string& path, const std::string& requestBody);
std::string ReadResponse(HINTERNET hRequest);
extern void Log(const std::string& message);
extern void LogDebug(const std::string& message);
std::string FetchDataFromAPI(const std::string& url);

extern std::string session_id;

static int apiSelection = 0;
const char* apiOptions[] = { "Production", "Development" };