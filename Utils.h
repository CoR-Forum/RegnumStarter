#ifndef UTILS_H
#define UTILS_H

#include <string>

HINSTANCE hInstanceGlobal;
HINSTANCE hInstance;

// Window procedures
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM); // Handles messages sent to the main window, such as creating and destroying windows, and custom messages like

// Pull some functions to the top
void MemoryManipulation(const std::string& option);
void OpenLoginWindow();

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

// Keydown states
bool isGravityKeyPressed = false;

// Seems to be the main window declaration?
HWND hwnd = nullptr;

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