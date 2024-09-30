#include <windows.h>
#include <urlmon.h>
#include <comdef.h>
#include <fstream>
#include <iostream>
#include <string>
#include <deque>
#include <ctime>
#include <cstdio>
#include <objbase.h> // Include for COM
#include <vector>    // Include for std::vector
#include <tlhelp32.h> // Include for CreateToolhelp32Snapshot and related functions
#include "Updater.cpp"
#include "Utils.h"
#include "resource.h"

#pragma comment(lib, "urlmon.lib")

// Define GUIDs for IID_IBindStatusCallback and IID_IUnknown
const IID IID_IBindStatusCallback = {0x79eac9c1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}};
const IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

// Constants
const std::string currentVersion = "0.1.14"; // Current version of the application
const char* appDataPath = getenv("APPDATA");
const char* appName = "Sylent-X";
const UINT WM_START_SELF_UPDATE = WM_USER + 1; // Custom message identifier

// Checkboxes states
bool optionNoclip = false;
bool optionSpeedhack = false;
bool optionZoom = false;

// Handle to the target process (ROClientGame.exe)
HANDLE hProcess = nullptr;

// Handle to the log display control
HWND hLogDisplay = nullptr;

// Deque to store the last 50 log messages
std::deque<std::string> logMessages;

// Handle to the background image
HBITMAP hBackgroundImage = nullptr;

// Declare global variables
DWORD pid;

// Function Prototypes
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void SaveSettings();
void LoadSettings();
void MemoryManipulation(HWND hwnd, bool isZoomEnabled); // Updated prototype
void UpdateLogDisplay();
void Log(const std::string& message);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Log("Sylent-X " + currentVersion + " started");
    Log("Made with hate in Germany");

    LoadSettings();  // Load saved settings on startup

    // Register the window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = appName;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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
    Log("Window created");

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
    static HBITMAP hBackgroundImage = NULL;

    switch (msg) {
        case WM_CREATE:
            Log("Creating checkboxes");

            // Load the background image from resources
            // Load the background image from resources
            hBackgroundImage = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BACKGROUND_BITMAP));
            if (!hBackgroundImage) {
                DWORD error = GetLastError();
                Log("Failed to load background image. Error code: " + std::to_string(error));
            } else {
                Log("Background image loaded successfully");
            }

            // Create checkboxes
            chkoptionNoclip = CreateWindow("BUTTON", "Enable Noclip", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 50, 150, 20, hwnd, (HMENU)1, NULL, NULL);
            chkoptionSpeedhack = CreateWindow("BUTTON", "Enable Speedhack", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 80, 150, 20, hwnd, (HMENU)2, NULL, NULL);
            chkoptionZoom = CreateWindow("BUTTON", "Enable Zoom", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 110, 150, 20, hwnd, (HMENU)3, NULL, NULL);
            // Create log display
            hLogDisplay = CreateWindow("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY,
                                       20, 200, 760, 100, hwnd, NULL, NULL, NULL);
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (hBackgroundImage) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBackgroundImage);

                BITMAP bm;
                GetObject(hBackgroundImage, sizeof(bm), &bm);
                BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

                SelectObject(hdcMem, hbmOld);
                DeleteDC(hdcMem);
            }

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
                MemoryManipulation(hwnd, SendMessage(chkoptionZoom, BM_GETCHECK, 0, 0) == BST_CHECKED);  // Pass hwnd and zoom state
            }
            if (LOWORD(wParam) == 2) {
                optionSpeedhack = !optionSpeedhack;
                SendMessage(chkoptionSpeedhack, BM_SETCHECK, optionSpeedhack ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Speedhack toggled");
                MemoryManipulation(hwnd, SendMessage(chkoptionZoom, BM_GETCHECK, 0, 0) == BST_CHECKED);  // Pass hwnd and zoom state
            }
            if (LOWORD(wParam) == 3) {
                optionZoom = !optionZoom;
                SendMessage(chkoptionZoom, BM_SETCHECK, optionZoom ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Zoom toggled");
                MemoryManipulation(hwnd, optionZoom);  // Pass hwnd and zoom state
            }
            break;

        case WM_DESTROY:
            Log("Saving settings");
            SaveSettings();  // Save settings on exit

            // Clean up the background image
            if (hBackgroundImage) {
                DeleteObject(hBackgroundImage);
            }

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
    std::ofstream file(std::string(appDataPath) + "\\Sylent-X\\settings.txt");
    file << "optionNoclip=" << optionNoclip << std::endl;
    file << "optionSpeedhack=" << optionSpeedhack << std::endl;
    file.close();
}

void LoadSettings() {
    Log("Loading settings from file");
    std::ifstream file(std::string(appDataPath) + "\\Sylent-X\\settings.txt");
    if (!file) {
        Log("Settings file not found");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("optionNoclip=") != std::string::npos)
            optionNoclip = (line.substr(line.find("=") + 1) == "1");
        if (line.find("optionSpeedhack=") != std::string::npos)
            optionSpeedhack = (line.substr(line.find("=") + 1) == "1");
    }
    file.close();
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
        Log("Process snapshot created for process ID: " + std::to_string(procId));
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                wchar_t wModuleName[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, modEntry.szModule, -1, wModuleName, MAX_PATH);
                Log("Checking module: " + std::string(modEntry.szModule));
                if (!_wcsicmp(wModuleName, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    Log("Module found: " + std::string(modEntry.szModule) + 
                        " at address: " + std::to_string(modBaseAddr) + 
                        " in process ID: " + std::to_string(procId));
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    } else {
        Log("Failed to create process snapshot for process ID: " + std::to_string(procId));
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

void MemoryManipulation(HWND hwnd, bool isZoomEnabled) {
    Log("Performing memory manipulation");

    pid = GetProcessIdByName(L"ROClientGame.exe");
    if (pid == 0) {
        Log("Failed to find ROClientGame.exe process");
        MessageBox(NULL, "Failed to find ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        Log("Failed to open ROClientGame.exe process");
        MessageBox(NULL, "Failed to open ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }

    Memory memory;
    MemoryAddress zoomAddr = {"Zoom", 0x007AE4CC, {0x88}};
    uintptr_t baseAddress = memory.GetBaseAddress(zoomAddr);

    if (isZoomEnabled) {
        if (memory.WriteFloat(baseAddress, 25.0f)) {
            Log("Zoom enabled.");
        } else {
            Log("Failed to write Zoom value to memory.");
        }
    } else {
        if (memory.WriteFloat(baseAddress, 15.0f)) {
            Log("Zoom disabled.");
        } else {
            Log("Failed to write Zoom value to memory.");
        }
    }

    CloseHandle(hProcess);
    Log("Memory manipulation completed");
}

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
    UpdateLogDisplay();
}

void UpdateLogDisplay() {
    if (hLogDisplay) {
        SendMessage(hLogDisplay, LB_RESETCONTENT, 0, 0);
        for (const auto& msg : logMessages) {
            SendMessage(hLogDisplay, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
        }
        SendMessage(hLogDisplay, LB_SETTOPINDEX, logMessages.size() - 1, 0);
    }
}