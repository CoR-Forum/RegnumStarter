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
#include "../libs/imgui/imfilebrowser.h"
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

extern void MemoryManipulation(const std::string& option, float newValue = 0.0f);
extern void RegisterUser(const std::string& username, const std::string& email, const std::string& password);
extern void LoadLoginCredentials(HINSTANCE hInstance);
extern void SaveLoginCredentials(const std::string& login, const std::string& encryptedPassword);
extern void SendChatMessage(const std::string& login, const std::string& password, const std::string& message);
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
DWORD GetProcessIdByName(const std::wstring& processName);
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

struct RegnumAccount {
    int id;
    std::string username;
    std::string password;
    std::string server;
    std::string referrer;
};

extern void SaveRegnumAccount(const std::string& username, const std::string& password, const std::string& server, const std::string& referrer, int id);
extern void LoadRegnumAccounts();
extern void DeleteRegnumAccount(int id);

struct ServerOption {
    const char* id;
    const char* name;
};

struct ReferrerOption {
    const char* id;
    const char* name;
};

// global variable to store the loaded regnum accounts
extern std::vector<RegnumAccount> regnumAccounts;

// Global constants
const char* appDataPath = getenv("APPDATA");
const std::string currentVersion = "1.1.63"; // Current version of the application
const std::string appName = "Sylent-X";

extern ImVec4 textColor;

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