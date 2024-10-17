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
#include <unordered_set>
#include <regex>
#include <tlhelp32.h>
#include <tchar.h>
#include <urlmon.h>
#include <comdef.h>
#include <objbase.h>
#include <wininet.h>
#include <shlobj.h>
#include <d3d9.h>
#include "../libs/json.hpp"
#include "../libs/imgui/imgui.h"
#include "./resource.h"
#include <locale>
#include <codecvt>
#include "../libs/imgui/imgui_impl_dx9.h"
#include "../libs/imgui/imgui_impl_win32.h"
#include "../libs/imgui/imgui_internal.h"
using json = nlohmann::json;

HINSTANCE hInstanceGlobal;
HINSTANCE hInstance;

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM); // Handles messages sent to the main window, such as creating and destroying windows, and custom messages like
const UINT WM_START_SELF_UPDATE = WM_USER + 1; // Custom message identifier for starting the self-update process
const UINT WM_ENABLE_CHECKBOXES = WM_USER + 3; // Message Identifier for retrieving message to enable checkboxes

struct Pointer {
    std::string name;
    unsigned long address;
    std::vector<unsigned long> offsets;
};

extern std::vector<Pointer> InitializePointers(); // Updated declaration
extern std::vector<Pointer> g_pointers;
extern std::vector<std::string> g_chatMessages;
extern std::string login;
extern std::string password;

extern void MemoryManipulation(const std::string& option, float newValue = 0.0f);
extern void RegisterUser(const std::string& username, const std::string& email, const std::string& password);
extern void LoadLoginCredentials(HINSTANCE hInstance);
extern void SaveLoginCredentials(const std::string& login, const std::string& encryptedPassword);
extern void SendChatMessage(const std::string& login, const std::string& password, const std::string& message);
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
DWORD GetProcessIdByName(const std::wstring& processName);
extern bool ResetPasswordRequest(const std::string& email);
extern void CheckChatMessages();
extern void GetAllUsers();
extern void DisplayUsersTable();
extern void ToggleUserBan(int userId);
extern void ToggleUserAdmin(int userId);
extern void ToggleUserActivation(int userId);
extern void GetMagnatCurrency();
extern void SendFeedback(const std::string& type, const std::string& feedback, bool feedback_includeLogfile);
extern void ActivateLicense(const std::string& licenseKey);
extern void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);

// variable to store current amount of Magnat currency for the user
int magnatCurrency;

// User settings
extern std::string login;
extern std::string password;

extern std::string license_runtime_end;
extern std::string license_features;

// Global constants
const char* appDataPath = getenv("APPDATA");
const std::string currentVersion = "0.1.63"; // Current version of the application
const char* appName = "Sylent-X";

extern ImVec4 textColor;

float setting_fontSize = 14.0f;
bool setting_enableRainbow = false;
float setting_rainbowSpeed = 0.1f;
bool setting_excludeFromCapture = false;

// Global variables
bool setting_log_debug = true;
int setting_log_maxMessages = 10;
bool isAdmin = false;

// Checkboxes states
bool optionGravity = false;
bool optionMoonjump = false;
bool optionZoom = false;
bool optionFreecam = false;
bool optionMoonwalk = false;
bool optionFov = false;
bool optionFastFly = false;
bool optionSpeedHack = false;
bool optionFakelag = false;

// Feature states
bool featureZoom = true;
bool featureFov = false;
bool featureGravity = false;
bool featureFreecam = false;
bool featureMoonwalk = false;
bool featureFakelag = false;
bool featureMoonjump = false;
bool featureFastfly = false;
bool featureSpeedhack = false;

// Keydown states
bool isGravityKeyPressed = false;

// Seems to be the main window declaration?
HWND hwnd = nullptr;

// Convert wstring to string
std::string WStringToString(const std::wstring& wstr) {
    std::string str(wstr.begin(), wstr.end());
    return str;
}

#endif // UTILS_H