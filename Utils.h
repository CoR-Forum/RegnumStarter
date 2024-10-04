#ifndef UTILS_H
#define UTILS_H

#include <string>

HINSTANCE hInstanceGlobal;
HINSTANCE hInstance;

void Log(const std::string& message);
void LogDebug(const std::string& message);

// Declare login and password globally
std::string login;
std::string password;

// Global variables
bool debugLog = false; //

// Checkboxes states
bool optionGravity = false;
bool optionMoonjump = false;
bool optionZoom = true;

// Feature states
bool featureZoom = false;
bool featureGravity = false;

// Declare the window handles
HWND hRegistrationWindow;
HWND hLoginWindow;

// Functions

// Convert wstring to string
std::string WStringToString(const std::wstring& wstr) {
    std::string str(wstr.begin(), wstr.end());
    return str;
}

#endif // UTILS_H