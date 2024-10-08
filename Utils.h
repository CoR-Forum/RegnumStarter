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
#include <utility>
#include <urlmon.h>
#include <comdef.h>
#include <objbase.h>
#include <wininet.h>
#include <shlobj.h>
#include "json.hpp"
using json = nlohmann::json;

HINSTANCE hInstanceGlobal;
HINSTANCE hInstance;

// Window procedures
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM); // Handles messages sent to the main window, such as creating and destroying windows, and custom messages like
const UINT WM_START_SELF_UPDATE = WM_USER + 1; // Custom message identifier for starting the self-update process
const UINT WM_ENABLE_CHECKBOXES = WM_USER + 3; // Message Identifier for retrieving message to enable checkboxes

// Declare the Pointer struct
struct Pointer {
    std::string name;
    unsigned long address;
    std::vector<unsigned long> offsets;
};

// Pull some functions to the top
void MemoryManipulation(const std::string& option, float newValue = 0.0f);
extern std::vector<Pointer> InitializePointers(); // Updated declaration
extern void LoadLoginCredentials(HINSTANCE hInstance);
extern void SaveLoginCredentials(const std::string& login, const std::string& encryptedPassword);
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
DWORD GetProcessIdByName(const std::wstring& processName);
void ContinuousMemoryWrite(const std::string& option);
extern bool ResetPasswordRequest(const std::string& email);

// Declare login and password globally
std::string login;
std::string password;

// extern std::vector<Pointer> pointers; // Declare pointers as an external global variable

// Define MemoryAddress struct
struct MemoryAddress {
    std::string name;
    uintptr_t address;
    std::vector<unsigned long> offsets;
};

// Global constants
const char* appDataPath = getenv("APPDATA");
extern const std::string currentVersion;

// Global variables
bool debugLog = true;

// Checkboxes states
bool optionGravity = false;
bool optionMoonjump = false;
bool optionZoom = true;
bool optionFreecam = false;
bool optionMoonwalk = false;

// Feature states
bool featureZoom = true;
bool featureGravity = false;
bool featureFreecam = false;
bool featureMoonwalk = false;
bool featureMoonjump = false;

// Keydown states
bool isGravityKeyPressed = false;

// Seems to be the main window declaration?
HWND hwnd = nullptr;

HWND hLogDisplay = nullptr; // Handle to the log display control

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