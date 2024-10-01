#include <windows.h>
#include <urlmon.h>
#include <comdef.h>
#include <fstream>
#include <iostream>
#include <string>
#include <deque>
#include <ctime>
#include <cstdio>
#include <objbase.h>
#include <vector>
#include <tlhelp32.h>
#include "Updater.cpp"
#include "Utils.h"
#include "Logger.cpp" // Include the combined Logger file
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")

// Define GUIDs for IID_IBindStatusCallback and IID_IUnknown
const IID IID_IBindStatusCallback = {0x79eac9c1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}};
const IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

// Constants
const std::string currentVersion = "0.1.16"; // Current version of the application
const char* appDataPath = getenv("APPDATA");
const char* appName = "Sylent-X";
const UINT WM_START_SELF_UPDATE = WM_USER + 1; // Custom message identifier

// Checkboxes states
bool optionNoclip = false;
bool optionSpeedhack = false;
bool optionZoom = false;

// Debug Log enabled
bool debugLog = true;

// Handle to the target process (ROClientGame.exe)
HANDLE hProcess = nullptr;

// Handle to the log display control
HWND hLogDisplay = nullptr;

// Deque to store the last 50 log messages
std::deque<std::string> logMessages;

// Declare global variables
DWORD pid;

// Function Prototypes
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void SaveSettings();
void LoadSettings();
void MemoryManipulation(const std::string& option); // Updated prototype
void UpdateLogDisplay();
void Log(const std::string& message);
void LogDebug(const std::string& message); // Renamed function

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Log("Sylent-X " + currentVersion + ". Made with hate in Germany.");

    LoadSettings();  // Load saved settings on startup

    // Register the window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = appName;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    if (!RegisterClassEx(&wc)) {
        Log("Failed to register window class");
        MessageBox(NULL, "Failed to register window class.", "Error", MB_ICONERROR);
        return 0;
    }

    // Create the window with the current version in the title
    std::string windowTitle = "Sylent-X " + currentVersion;
    HWND hwnd = CreateWindowEx(0, appName, windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 756, 504, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        Log("Failed to create window");
        MessageBox(NULL, "Failed to create window.", "Error", MB_ICONERROR);
        return 0;
    }
    LogDebug("Window created");

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Post custom message to start self-update
    PostMessage(hwnd, WM_START_SELF_UPDATE, 0, 0);

    // Message loop
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Log("Sylent-X exiting");
    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND chkoptionNoclip, chkoptionSpeedhack, chkoptionZoom;
    static HINSTANCE hInstance = GetModuleHandle(NULL);

    switch (msg) {
        case WM_CREATE:
            LogDebug("Creating checkboxes");

            // Create checkboxes UI elements
            chkoptionNoclip = CreateWindow("BUTTON", "Enable Noclip", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 50, 150, 20, hwnd, (HMENU)1, NULL, NULL);
            chkoptionSpeedhack = CreateWindow("BUTTON", "Enable Speedhack", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 80, 150, 20, hwnd, (HMENU)2, NULL, NULL);
            chkoptionZoom = CreateWindow("BUTTON", "Enable Zoom", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 110, 150, 20, hwnd, (HMENU)3, NULL, NULL);
            hLogDisplay = CreateWindow("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY,
                                       20, 200, 760, 100, hwnd, NULL, NULL, NULL);

            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Fill the background with the custom color
            HBRUSH hBrush = CreateSolidBrush(RGB(1, 1, 1)); // Custom background color (white)
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_SIZE:
            if (hLogDisplay) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                int logHeight = 100;
                SetWindowPos(hLogDisplay, NULL, 20, rect.bottom - logHeight - 20, rect.right - 40, logHeight, SWP_NOZORDER);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                optionNoclip = !optionNoclip;
                SendMessage(chkoptionNoclip, BM_SETCHECK, optionNoclip ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Noclip toggled");
                MemoryManipulation("noclip");
            }
            if (LOWORD(wParam) == 2) {
                optionSpeedhack = !optionSpeedhack;
                SendMessage(chkoptionSpeedhack, BM_SETCHECK, optionSpeedhack ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Speedhack toggled");
                MemoryManipulation("speedhack");
            }
            if (LOWORD(wParam) == 3) {
                optionZoom = !optionZoom;
                SendMessage(chkoptionZoom, BM_SETCHECK, optionZoom ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Zoom toggled");
                MemoryManipulation("zoom");
            }
            break;

        case WM_DESTROY:
            Log("Saving settings");
            SaveSettings();  // Save settings on exit

            PostQuitMessage(0);
            break;

        case WM_START_SELF_UPDATE:
            SelfUpdate();
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

void SaveSettings() {
    Log("Saving settings to file");

    // Construct the settings file path
    std::string settingsDir = std::string(appDataPath) + "\\Sylent-X";
    std::string settingsFilePath = settingsDir + "\\settings.txt";

    // Create the directory if it doesn't exist
    CreateDirectory(settingsDir.c_str(), NULL);

    // Open the file and write the settings
    std::ofstream file(settingsFilePath);
    if (file.is_open()) {
        file << "optionNoclip=" << optionNoclip << std::endl;
        file << "optionSpeedhack=" << optionSpeedhack << std::endl;
        file << "optionZoom=" << optionZoom << std::endl;
        file.close();
        Log("Settings saved successfully");
    } else {
        Log("Failed to open settings file for writing");
    }
}

void LoadSettings() {
    LogDebug("Loading settings from file");

    // Construct the settings file path
    std::string settingsFilePath = std::string(appDataPath) + "\\Sylent-X\\settings.txt";

    // Open the file and read the settings
    std::ifstream file(settingsFilePath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("optionNoclip=") != std::string::npos)
                optionNoclip = (line.substr(line.find("=") + 1) == "1");
            if (line.find("optionSpeedhack=") != std::string::npos)
                optionSpeedhack = (line.substr(line.find("=") + 1) == "1");
            if (line.find("optionZoom=") != std::string::npos)
                optionZoom = (line.substr(line.find("=") + 1) == "1");
        }
        file.close();
        Log("Settings loaded successfully");
    } else {
        LogDebug("Settings file not found");
    }
}

// Define MemoryAddress struct
struct MemoryAddress {
    std::string name;
    uintptr_t baseOffset;
    std::vector<uintptr_t> offsets;
};

// Function to get the base address of a module
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE) {
        LogDebug("Process snapshot created for process ID: " + std::to_string(procId));
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                wchar_t wModuleName[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, modEntry.szModule, -1, wModuleName, MAX_PATH);
                LogDebug("Checking module: " + std::string(modEntry.szModule));
                if (!_wcsicmp(wModuleName, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    LogDebug("Module found: " + std::string(modEntry.szModule) + 
                        " at address: " + std::to_string(modBaseAddr) + 
                        " in process ID: " + std::to_string(procId));
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    } else {
        LogDebug("Failed to create process snapshot for process ID: " + std::to_string(procId));
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

// Function to get the process ID by name
DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnap, &pe32)) {
            do {
                // Convert pe32.szExeFile to a wide string
                std::wstring exeFile(pe32.szExeFile, pe32.szExeFile + strlen(pe32.szExeFile));
                if (!_wcsicmp(exeFile.c_str(), processName.c_str())) {
                    processId = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pe32));
        }
    }
    CloseHandle(hSnap);
    return processId;
}

class Memory {
public:
    uintptr_t GetBaseAddress(const MemoryAddress& memAddr);
    bool WriteFloat(uintptr_t address, float value);
};

uintptr_t Memory::GetBaseAddress(const MemoryAddress& memAddr) {
    return GetModuleBaseAddress(pid, L"ROClientGame.exe") + memAddr.baseOffset;
}

bool Memory::WriteFloat(uintptr_t address, float value) {
    return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(value), NULL);
}

struct Pointer {
    std::string name;
    uintptr_t address;
    std::vector<uintptr_t> offsets;
};

// Vector to store pointers to memory addresses
std::vector<Pointer> pointers;

#include <sstream>
#include <iomanip>

void MemoryManipulation(const std::string& option) {
    LogDebug("Performing memory manipulation for " + option);

    // Get process ID for ROClientGame.exe
    pid = GetProcessIdByName(L"ROClientGame.exe");
    if (pid == 0) {
        Log("Failed to find ROClientGame.exe process");
        MessageBox(NULL, "Failed to find ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }
    LogDebug("Process ID for ROClientGame.exe: " + std::to_string(pid));

    // Open the process with full access
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        Log("Failed to open ROClientGame.exe process. Error code: " + std::to_string(GetLastError()));
        MessageBox(NULL, "Failed to open ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }
    LogDebug("Successfully opened process with ID: " + std::to_string(pid));

    // Get the base address of ROClientGame.exe
    uintptr_t baseAddress = GetModuleBaseAddress(pid, L"ROClientGame.exe");
    if (baseAddress == 0) {
        Log("Failed to get the base address of ROClientGame.exe");
        CloseHandle(hProcess);
        return;
    }
    LogDebug("Base address of ROClientGame.exe: " + std::to_string(baseAddress));

    // Use the fetched pointers
    for (const auto& pointer : pointers) {
        if (pointer.name == option) {
            uintptr_t optionPointer = baseAddress + pointer.address;
            LogDebug(option + " pointer address: " + std::to_string(optionPointer));

            float newValue = 0.0f;

            if (option == "zoom") {
                newValue = optionZoom ? 25.0f : 15.0f;
            } else if (option == "speedhack") {
                newValue = optionSpeedhack ? 25.0f : 15.0f;
            }

            // Concatenate offsets into a single string
            std::string optionOffsets;
            for (const auto& offset : pointer.offsets) {
                optionOffsets += std::to_string(offset) + ",";
            }
            // Remove the trailing comma
            if (!optionOffsets.empty()) {
                optionOffsets.pop_back();
            }

            // Log the offsets string
            LogDebug("Offsets string: " + optionOffsets);

            // Use the string directly
            uintptr_t finalAddress = optionPointer + std::stoul(optionOffsets, nullptr, 16);

            // Convert finalAddress to hexadecimal string
            std::stringstream ss;
            ss << std::hex << std::uppercase << finalAddress;
            std::string finalAddressHex = ss.str();

            // Log the final address and value being written
            LogDebug("Writing value: " + std::to_string(newValue) + " to address: " + finalAddressHex);

            if (WriteProcessMemory(hProcess, (LPVOID)finalAddress, &newValue, sizeof(newValue), NULL)) {
                LogDebug("Successfully wrote new " + option + " value: " + std::to_string(newValue));
            } else {
                LogDebug("Failed to write new " + option + " value. Error code: " + std::to_string(GetLastError()));
            }
        }
    }

    // Close the process handle
    CloseHandle(hProcess);
    LogDebug("Memory manipulation completed");
}

std::string FetchDataFromAPI(const std::string& url) {
    HINTERNET hInternet = InternetOpen("Sylent-X", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        Log("Failed to open internet connection");
        return "";
    }

    HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        Log("Failed to open URL");
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[4096];
    DWORD bytesRead;
    std::string response;

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
        response.append(buffer, bytesRead);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}

std::vector<Pointer> ParseJSONResponse(const std::string& jsonResponse) {
    std::vector<Pointer> pointers;
    size_t pos = 0, endPos;

    while ((pos = jsonResponse.find("{", pos)) != std::string::npos) {
        Pointer pointer;
        endPos = jsonResponse.find("}", pos);
        std::string object = jsonResponse.substr(pos, endPos - pos + 1);

        size_t namePos = object.find("\"name\":") + 8;
        size_t nameEnd = object.find("\"", namePos);
        pointer.name = object.substr(namePos, nameEnd - namePos);

        size_t addressPos = object.find("\"address\":") + 11;
        size_t addressEnd = object.find("\"", addressPos);
        std::string addressStr = object.substr(addressPos, addressEnd - addressPos);
        try {
            pointer.address = std::stoul(addressStr, nullptr, 16);
        } catch (const std::invalid_argument& e) {
            LogDebug("Invalid address: " + addressStr);
            continue;
        }

        size_t offsetsPos = object.find("\"offsets\":") + 11;
        size_t offsetsEnd = object.find("\"", offsetsPos);
        std::string offsetsStr = object.substr(offsetsPos, offsetsEnd - offsetsPos);
        LogDebug("Offsets string: " + offsetsStr); // Add this line to log the offsets string

        if (offsetsStr.empty()) {
            LogDebug("No offsets for pointer: " + pointer.name);
        } else {
            size_t offsetPos = 0, offsetEnd;
            while ((offsetEnd = offsetsStr.find(",", offsetPos)) != std::string::npos) {
                std::string offsetStr = offsetsStr.substr(offsetPos, offsetEnd - offsetPos);
                try {
                    pointer.offsets.push_back(std::stoul(offsetStr, nullptr, 16));
                    LogDebug("Offset: " + offsetStr); // Add this line to log the offset
                } catch (const std::invalid_argument& e) {
                    LogDebug("Invalid offset: " + offsetStr);
                    continue;
                }
                offsetPos = offsetEnd + 1;
            }
            try {
                pointer.offsets.push_back(std::stoul(offsetsStr.substr(offsetPos), nullptr, 16));
                LogDebug("Last offset: " + offsetsStr.substr(offsetPos)); // Add this line to log the last offset
            } catch (const std::invalid_argument& e) {
                LogDebug("Invalid offset: " + offsetsStr.substr(offsetPos));
                continue;
            }
        }

        // Log the fetched data
        LogDebug("Fetched pointer: Name = " + pointer.name + ", Address = " + std::to_string(pointer.address));

        pointers.push_back(pointer);
        pos = endPos + 1;
    }

    return pointers;
}

void InitializePointers() {
    std::string url = "https://cort.cor-forum.de/api/v1/sylentx/memory/pointers?key=aingu8gaiv0daitoj6eeweezeug7Ei";
    std::string jsonResponse = FetchDataFromAPI(url);
    if (!jsonResponse.empty()) {
        pointers = ParseJSONResponse(jsonResponse);
        Log("Pointers fetched and parsed successfully");
    } else {
        Log("Failed to fetch or parse pointers");
    }
}