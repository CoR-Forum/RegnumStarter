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
#include "resource.h"
#include "Logger.cpp" // Include the combined Logger file

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

// Debug Log enabled
bool debugLog = true;

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
void LogDebug(const std::string& message); // Renamed function

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
    static HBITMAP hBackgroundImage = NULL;

    switch (msg) {
        case WM_CREATE:
            LogDebug("Creating checkboxes");

            // Load the background image from resources
            hBackgroundImage = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BACKGROUND_BITMAP));
            if (!hBackgroundImage) {
                DWORD error = GetLastError();
                LogDebug("Failed to load background image. Error code: " + std::to_string(error));
            } else {
                LogDebug("Background image loaded successfully");
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
    LogDebug("Loading settings from file");
    std::ifstream file(std::string(appDataPath) + "\\Sylent-X\\settings.txt");
    if (!file) {
        LogDebug("Settings file not found");
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

void MemoryManipulation(HWND hwnd, bool isZoomEnabled) {
    LogDebug("Performing memory manipulation");

    // Get process ID for ROClientGame.exe
    pid = GetProcessIdByName(L"ROClientGame.exe");
    if (pid == 0) {
        Log("Failed to find ROClientGame.exe process");
        MessageBox(NULL, "Failed to find ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }
    LogDebug("Process ID for ROClientGame.exe: " + std::to_string(pid));

    // Open the process with write and operation access
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

    // Directly read from the specified pointer with the offset
    uintptr_t zoomPointer = baseAddress + 0x007AE4CC;
    LogDebug("Zoom pointer address: " + std::to_string(zoomPointer));

    // To read the address stored at that location
    uintptr_t zoomAddress; // This will hold the address where the zoom value is stored
    SIZE_T bytesRead;

    // Read the memory at the zoomPointer to get the address of the zoom value
    if (ReadProcessMemory(hProcess, (LPCVOID)zoomPointer, &zoomAddress, sizeof(zoomAddress), &bytesRead)) {
        if (bytesRead == sizeof(zoomAddress)) {
            LogDebug("Successfully read zoom address: " + std::to_string(zoomAddress));

            // Determine the new zoom value based on the checkbox state
            float newZoomValue = isZoomEnabled ? 25.0f : 15.0f;

            // Write the new zoom value to the memory location
            if (WriteProcessMemory(hProcess, (LPVOID)(zoomAddress + 0x88), &newZoomValue, sizeof(newZoomValue), NULL)) {
                LogDebug("Successfully wrote new zoom value: " + std::to_string(newZoomValue));
            } else {
                LogDebug("Failed to write new zoom value. Error code: " + std::to_string(GetLastError()));
            }
        } else {
            LogDebug("Failed to read the zoom pointer address. Bytes read: " + std::to_string(bytesRead));
        }
    } else {
        LogDebug("Failed to read zoom pointer from memory. Error code: " + std::to_string(GetLastError()));
    }

    // Close the process handle
    CloseHandle(hProcess);
    LogDebug("Memory read completed");
}