#pragma once

#include <string>
#include <deque>
#include <mutex>
#include <fstream>
#include <iostream>
#include <chrono>
#include <locale>
#include <codecvt>
#include "../Utils.h"

// External variables
extern bool setting_log_debug;

void Log(const std::string& message);

// for some reason those 2 are needed
void LogDebug(const std::string& message);
void LogDebug(const std::wstring& message);

extern const char* appDataPath;