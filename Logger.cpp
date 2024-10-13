#include "Logger.h"

namespace {
    const size_t MAX_LOG_MESSAGES = 500;
    const char* LOG_FILE_PATH = "\\Sylent-X\\log.txt";
    std::deque<std::string> logMessages;
    std::mutex logMutex;
}

void WriteLogToFile(const std::string& logMessage) {
    std::ofstream logFile(std::string(appDataPath) + LOG_FILE_PATH, std::ios_base::app);
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
    } else {
        std::cerr << "Failed to open log file: " << std::string(appDataPath) + LOG_FILE_PATH << std::endl;
    }
}

std::string GetCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(timestamp);
}

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

void LogDebug(const std::string& message) {
    if (setting_debugLog) {
        Log("DEBUG: " + message);
    }
}

std::string WStringToStringForLog(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

void LogDebug(const std::wstring& message) {
    LogDebug(WStringToStringForLog(message));
}