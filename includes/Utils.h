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


std::vector<float> ReadMemoryValues(const std::vector<std::string>& options);

extern void MemoryManipulation(const std::string& option, float newValue = 0.0f);
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
DWORD GetProcessIdByName(const std::wstring& processName);
extern void GetAllUsers();

// Global constants
const char* appDataPath = getenv("APPDATA");
const std::string sylentx_version = "0.1.66";
const std::string sylentx_appname = "Sylent-X";

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