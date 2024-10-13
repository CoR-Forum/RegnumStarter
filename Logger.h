#pragma once

#include "includes/Utils.h"
#include <string>
#include <deque>
#include <mutex>
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <locale>
#include <codecvt>

extern bool setting_debugLog;
extern const char* appDataPath;

void WriteLogToFile(const std::string& logMessage);
std::string GetCurrentTimestamp();
void Log(const std::string& message);
void LogDebug(const std::string& message);
std::string WStringToStringForLog(const std::wstring& wstr);
void LogDebug(const std::wstring& message);