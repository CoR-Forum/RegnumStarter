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
extern bool setting_log_debug; ///< Flag to enable or disable debug logging

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
 * @brief Logs a debug message in wide string format if debug logging is enabled.
 * 
 * @param message The wide string debug message to be logged.
 */
void LogDebug(const std::wstring& message);

extern const char* appDataPath;