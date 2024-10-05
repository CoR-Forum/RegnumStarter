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
#include <mutex>
#include <urlmon.h>
#include <comdef.h>
#include <objbase.h>
#include <wininet.h>

HINSTANCE hInstanceGlobal;
HINSTANCE hInstance;

// Window procedures
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM); // Handles messages sent to the main window, such as creating and destroying windows, and custom messages like
const UINT WM_START_SELF_UPDATE = WM_USER + 1; // Custom message identifier for starting the self-update process
const UINT WM_ENABLE_CHECKBOXES = WM_USER + 3; // Message Identifier for retrieving message to enable checkboxes

// Pull some functions to the top
void MemoryManipulation(const std::string& option);
void InitializePointers();
extern void LoadLoginCredentials(HINSTANCE hInstance);
extern void SaveLoginCredentials(const std::string& login, const std::string& encryptedPassword);
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
DWORD GetProcessIdByName(const std::wstring& processName);
void ContinuousMemoryWrite(const std::string& option);


// Declare login and password globally
std::string login;
std::string password;

// Define MemoryAddress struct
struct MemoryAddress {
    std::string name;
    uintptr_t baseOffset;
    std::vector<uintptr_t> offsets;
};

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

// Window handlers
HWND hRegistrationWindow;
HWND hLoginWindow;

// Functions for login and registration windows
void CreateLoginWindow(HINSTANCE hInstance);
void OpenLoginWindow();
void CreateRegistrationWindow(HINSTANCE hInstance);

// Convert wstring to string
std::string WStringToString(const std::wstring& wstr) {
    std::string str(wstr.begin(), wstr.end());
    return str;
}

#endif // UTILS_H