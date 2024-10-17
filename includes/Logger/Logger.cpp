#include "Logger.h"
#include <filesystem>

namespace {
    const size_t MAX_LOG_MESSAGES = setting_log_maxMessages; ///< Maximum number of log messages to keep in memory.
    const char* LOG_FILE_PATH = "\\Sylent-X\\log.txt"; ///< Path to the log file.
    std::deque<std::string> logMessages; ///< Deque to store log messages.
    std::mutex logMutex; ///< Mutex to protect access to logMessages.
}

/**
 * @brief Writes a log message to the log file.
 * 
 * @param logMessage The log message to write.
 */
void WriteLogToFile(const std::string& logMessage) {
    std::ofstream logFile(std::filesystem::path(appDataPath) / LOG_FILE_PATH, std::ios_base::app);
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
    } else {
        std::cerr << "Failed to open log file: " << std::filesystem::path(appDataPath) / LOG_FILE_PATH << std::endl;
    }
}

/**
 * @brief Gets the current timestamp in the format YYYY-MM-DD HH:MM:SS.
 * 
 * @return The current timestamp as a string.
 */
std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
    return std::string(timestamp);
}

/**
 * @brief Logs a message with a timestamp.
 * 
 * @param message The message to log.
 */
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

/**
 * @brief Logs a debug message if debug logging is enabled.
 * 
 * @param message The debug message to log.
 */
void LogDebug(const std::string& message) {
    if (setting_log_debug) {
        Log("DEBUG: " + message);
    }
}

/**
 * @brief Converts a wide string to a UTF-8 encoded string for logging.
 * 
 * @param wstr The wide string to convert.
 * @return The converted UTF-8 string.
 */
std::string WStringToStringForLog(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

/**
 * @brief Logs a debug message from a wide string if debug logging is enabled.
 * 
 * @param message The wide string debug message to log.
 */
void LogDebug(const std::wstring& message) {
    LogDebug(WStringToStringForLog(message));
}