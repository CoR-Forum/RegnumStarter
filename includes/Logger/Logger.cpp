#include "Logger.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <deque>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <locale>
#include <codecvt>

namespace {
    const std::filesystem::path LOG_FILE_PATH = "Sylent-X/log.txt"; ///< Path to the log file.
    std::deque<std::string> logMessages; ///< Deque to store log messages.
    std::mutex logMutex; ///< Mutex to protect access to logMessages.
}

/**
 * @brief Writes a log message to the log file.
 * 
 * @param logMessage The log message to write.
 */
void WriteLogToFile(const std::string& logMessage) {
    std::filesystem::path logFilePath = std::filesystem::path(appDataPath) / LOG_FILE_PATH;
    
    try {
        // Read the current log file into a deque
        std::deque<std::string> fileLines;
        {
            std::ifstream inFile(logFilePath);
            std::string line;
            while (std::getline(inFile, line)) {
                fileLines.push_back(line);
            }
        }

        // If the log file exceeds 1000 lines, remove the oldest lines
        while (fileLines.size() >= 1000) {
            fileLines.pop_front();
        }

        // Write the updated log lines back to the file
        {
            std::ofstream outFile(logFilePath, std::ios_base::trunc);
            for (const auto& logLine : fileLines) {
                outFile << logLine << std::endl;
            }
            outFile << logMessage << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to write to log file: " << logFilePath << ". Error: " << e.what() << std::endl;
    }
}

std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_c); // Thread-safe version of localtime
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Log(const std::string& message) {
    std::string logMessage = "[" + GetCurrentTimestamp() + "] " + message;

    {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back(logMessage);
        if (logMessages.size() > 1000) {
            logMessages.pop_front();
        }
    }

    WriteLogToFile(logMessage);
    std::cout << logMessage << std::endl;
}

void LogDebug(const std::string& message) {
    if (setting_log_debug) {
        Log("DEBUG: " + message);
    }
}

// Overload for LogDebug that accepts std::wstring
void LogDebug(const std::wstring& message) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string narrowMessage = converter.to_bytes(message);
    LogDebug(narrowMessage);
}