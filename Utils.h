#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <string>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <deque>
#include <sstream>
#include <thread>
#include <atomic>
#include <sstream>
#include <urlmon.h>
#include <comdef.h>
#include <objbase.h>
#include <wininet.h>

HINSTANCE hInstanceGlobal;
HINSTANCE hInstance;

// Window procedures
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM); // Handles messages sent to the main window, such as creating and destroying windows, and custom messages like

// Pull some functions to the top
void MemoryManipulation(const std::string& option);
void OpenLoginWindow();
void InitializePointers();
void CreateLoginWindow(HINSTANCE hInstance);
void CreateRegistrationWindow(HINSTANCE hInstance);
extern void LoadLoginCredentials(HINSTANCE hInstance);
extern void SaveLoginCredentials(const std::string& login, const std::string& encryptedPassword);

// Declare login and password globally
std::string login;
std::string password;

// Global constants
const char* appDataPath = getenv("APPDATA");
extern const std::string currentVersion;

// Global variables
bool debugLog = false;

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