#pragma once

#include "../Utils.h"
#include <string>
#include <deque>
#include <mutex>
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <locale>
#include <codecvt>

// External variables
extern bool setting_debugLog; ///< Flag to enable or disable debug logging
extern const char* appDataPath; ///< Path to the application data directory

/**
 * @brief Writes a log message to a file.
 * 
 * @param logMessage The message to be logged.
 */
void WriteLogToFile(const std::string& logMessage);

/**
 * @brief Gets the current timestamp in a string format.
 * 
 * @return A string representing the current timestamp.
 */
std::string GetCurrentTimestamp();

/**
 * @brief Logs a message.
 * 
 * @param message The message to be logged.
 */
void Log(const std::string& message);

/**
 * @brief Logs a debug message if debug logging is enabled.
 * 
 * @param message The debug message to be logged.
 */
void LogDebug(const std::string& message);

/**
 * @brief Converts a wide string to a standard string for logging purposes.
 * 
 * @param wstr The wide string to be converted.
 * @return A standard string representation of the wide string.
 */
std::string WStringToStringForLog(const std::wstring& wstr);

/**
 * @brief Logs a debug message in wide string format if debug logging is enabled.
 * 
 * @param message The wide string debug message to be logged.
 */
void LogDebug(const std::wstring& message);