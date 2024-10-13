#pragma once

#include "includes/Utils.h"

extern bool setting_debugLog;
extern const char* appDataPath;

namespace {
    const size_t MAX_LOG_MESSAGES = 500;
    const char* LOG_FILE_PATH = "\\Sylent-X\\log.txt";
    std::deque<std::string> logMessages;
    std::mutex logMutex;
}

// Writes a log message to the log file
void WriteLogToFile(const std::string& logMessage) {
    std::ofstream logFile(std::string(appDataPath) + LOG_FILE_PATH, std::ios_base::app);
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file: " << std::string(appDataPath) + LOG_FILE_PATH << std::endl;
    }
}

// Gets the current timestamp in the format YYYY-MM-DD HH:MM:SS
std::string GetCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(timestamp);
}

// Logs a message with a timestamp, writes it to the file, and updates the display
void Log(const std::string& message) {
    std::string logMessage = "[" + GetCurrentTimestamp() + "] " + message;

    {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back(logMessage);
        if (logMessages.size() > MAX_LOG_MESSAGES) {
            logMessages.pop_front();
        }
    }

    WriteLogToFile(logMessage);
    std::cout << logMessage << std::endl;
}

// Logs a debug message if debug logging is enabled
void LogDebug(const std::string& message) {
    if (setting_debugLog) {
        Log("DEBUG: " + message);
    }
}

// Converts std::wstring to std::string
std::string WStringToStringForLog(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Overloaded LogDebug to handle std::wstring
void LogDebug(const std::wstring& message) {
    LogDebug(WStringToStringForLog(message));
}