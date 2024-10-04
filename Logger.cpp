#ifndef LOGGER_H
#define LOGGER_H

#include "Utils.h"
#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <ctime>
#include <windows.h>

extern const char* appDataPath;
extern bool debugLog;
extern HWND hLogDisplay;


// Deque to store log messages
std::deque<std::string> logMessages;

void Log(const std::string& message) {
    // Get current time
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    // Create log message with timestamp
    std::string logMessage = "[" + std::string(timestamp) + "] " + message;

    std::ofstream logFile(std::string(appDataPath) + "\\Sylent-X\\log.txt", std::ios_base::app);
    logFile << logMessage << std::endl;
    logFile.close();
    std::cout << logMessage << std::endl;

    // Add message to deque
    logMessages.push_back(logMessage);
    if (logMessages.size() > 500) {
        logMessages.pop_front();
    }

    // Update the log display in the GUI
    if (hLogDisplay) {
        SendMessage(hLogDisplay, LB_RESETCONTENT, 0, 0);
        for (const auto& msg : logMessages) {
            SendMessage(hLogDisplay, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
        }
        SendMessage(hLogDisplay, LB_SETTOPINDEX, logMessages.size() - 1, 0);
    }
}

void LogDebug(const std::string& message) {
    if (debugLog) {
        Log("DEBUG: " + message);
    }
}

#endif // LOGGER_H