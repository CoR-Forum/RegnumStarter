// InternetUtils.h
#ifndef INTERNET_UTILS_H
#define INTERNET_UTILS_H

#include <windows.h>
#include <wininet.h>
#include <string>

void CloseInternetHandles(HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet);
bool LogAndCloseHandles(const std::string& message, HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet);
HINTERNET OpenInternetConnection();
HINTERNET ConnectToAPI(HINTERNET hInternet);
HINTERNET SendHTTPRequest(HINTERNET hConnect, const std::string& path);
std::string ReadResponse(HINTERNET hRequest);

extern void Log(const std::string& message);
extern void LogDebug(const std::string& message);

#endif // INTERNET_UTILS_H