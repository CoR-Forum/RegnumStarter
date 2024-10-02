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
#include <thread>
#include <atomic>
#include <sstream>
#include <iomanip>
#include "ApiFetcher.cpp" // Include the new ApiFetcher file
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")

std::atomic<bool> isWriting(false);
std::thread memoryThread;

void MemoryManipulation(const std::string& option); // Updated prototype

void ContinuousMemoryWrite(const std::string& option) {
    while (isWriting) {
        MemoryManipulation(option);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust the interval as needed
    }
}

// Define GUIDs for IID_IBindStatusCallback and IID_IUnknown
const IID IID_IBindStatusCallback = {0x79eac9c1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}};
const IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
const UINT WM_ENABLE_CHECKBOXES = WM_USER + 2; // New custom message identifier

// Constants
const std::string currentVersion = "1.1.27"; // Current version of the application
const char* appDataPath = getenv("APPDATA");
const char* appName = "Sylent-X";
const UINT WM_START_SELF_UPDATE = WM_USER + 1; // Custom message identifier

// Checkboxes states
bool optionGravity = false;
bool optionMoonjump = false;
bool optionZoom = false;

// license status
bool featureZoom = false;
bool featureGravity = false;

// Add a new boolean variable to track the state of the key
bool isGravityKeyPressed = false;

bool debugLog = true; // Debug Log enabled

HANDLE hProcess = nullptr; // Handle to the target process (ROClientGame.exe)

HWND hLogDisplay = nullptr; // Handle to the log display control

HWND hwnd = nullptr; // Declare hwnd globally to be accessible

std::deque<std::string> logMessages; // Deque to store log

DWORD pid; // Process ID of the target process

// Function Prototypes
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void SaveSettings();
void LoadSettings();
void UpdateLogDisplay();
void Log(const std::string& message);
void LogDebug(const std::string& message); // Renamed function
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam); // Added prototype

// Function prototypes
LRESULT CALLBACK LoginWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SaveLoginCredentials(const std::string& login, const std::string& encryptedPassword);
std::string EncryptPasswordMD5(const std::string& password);

// Global variables for login window
HWND hLogin, hPassword, hLoginButton;

void CreateLoginWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = LoginWindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = "LoginWindowClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        "LoginWindowClass",
        "Login",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
}

LRESULT CALLBACK LoginWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            CreateWindow("STATIC", "Username or E-Mail:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 25, hwnd, NULL, NULL, NULL);
            hLogin = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 20, 150, 25, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 60, 80, 25, hwnd, NULL, NULL, NULL);
            hPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD, 100, 60, 150, 25, hwnd, NULL, NULL, NULL);

            hLoginButton = CreateWindow("BUTTON", "Login", WS_VISIBLE | WS_CHILD, 100, 100, 80, 25, hwnd, (HMENU)1, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                char login[100], password[100];
                GetWindowText(hLogin, login, 100);
                GetWindowText(hPassword, password, 100);

                SaveLoginCredentials(login, password);

                Log("Login credentials saved");
                DestroyWindow(hwnd);
            }
            break;

        case WM_DESTROY:
            // PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Global hook handle
HHOOK hKeyboardHook;

HINSTANCE hInstanceGlobal;

void LoadLoginCredentials(HINSTANCE hInstance);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInstanceGlobal = hInstance; // Assign to global variable

    Log("Sylent-X " + currentVersion + ". Made with hate in Germany.");

    LoadSettings();

    LoadLoginCredentials(hInstance);

    if (!Login(login, password)) {
        Log("Login failed. Opening login window.");
        CreateLoginWindow(hInstance); // Open login window if login fails
    }

    // print license status
    Log("Feature Zoom: " + std::to_string(featureZoom));
    Log("Feature Gravity: " + std::to_string(featureGravity));

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

    // Set the global keyboard hook
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    if (!hKeyboardHook) {
        Log("Failed to set keyboard hook");
        MessageBox(NULL, "Failed to set keyboard hook.", "Error", MB_ICONERROR);
        return 0;
    }
    LogDebug("Keyboard hook set successfully");

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

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN && p->vkCode == VK_SPACE) {
            if (!isGravityKeyPressed) {
                isGravityKeyPressed = true;
                if (optionGravity) {
                    isWriting = true;
                    memoryThread = std::thread(ContinuousMemoryWrite, "gravity");
                }
            }
        } else if (wParam == WM_KEYUP && p->vkCode == VK_SPACE) {
            if (isGravityKeyPressed) {
                isGravityKeyPressed = false;
                isWriting = false;
                if (memoryThread.joinable()) {
                    memoryThread.join();
                }
            }
        }

        if (wParam == WM_KEYDOWN && p->vkCode == VK_LCONTROL) {
            if (!isGravityKeyPressed) {
                isGravityKeyPressed = true;
                if (optionGravity) {
                    isWriting = true;
                    memoryThread = std::thread(ContinuousMemoryWrite, "gravitydown");
                }
            }
        } else if (wParam == WM_KEYUP && p->vkCode == VK_LCONTROL) {
            if (isGravityKeyPressed) {
                isGravityKeyPressed = false;
                isWriting = false;
                if (memoryThread.joinable()) {
                    memoryThread.join();
                }
            }
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND chkoptionGravity, chkoptionMoonjump, chkoptionZoom;
    static HINSTANCE hInstance = GetModuleHandle(NULL);

    switch (msg) {
        case WM_CREATE:
            LogDebug("Creating checkboxes");

            // Create checkboxes UI elements
            chkoptionGravity = CreateWindow("BUTTON", "Enable Gravity", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 50, 150, 20, hwnd, (HMENU)1, NULL, NULL);
            chkoptionMoonjump = CreateWindow("BUTTON", "Enable Moonjump", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 80, 150, 20, hwnd, (HMENU)2, NULL, NULL);
            chkoptionZoom = CreateWindow("BUTTON", "Enable Zoom", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 110, 150, 20, hwnd, (HMENU)3, NULL, NULL);
            hLogDisplay = CreateWindow("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY,
                                       20, 200, 760, 100, hwnd, NULL, NULL, NULL);

            // Disable checkboxes by default
            EnableWindow(chkoptionGravity, FALSE);
            EnableWindow(chkoptionMoonjump, FALSE);
            EnableWindow(chkoptionZoom, FALSE);
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
                optionGravity = !optionGravity;
                SendMessage(chkoptionGravity, BM_SETCHECK, optionGravity ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Gravity toggled");
            }
            if (LOWORD(wParam) == 2) {
                optionMoonjump = !optionMoonjump;
                SendMessage(chkoptionMoonjump, BM_SETCHECK, optionMoonjump ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Moonjump toggled");
                MemoryManipulation("moonjump");
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
            SendMessage(hwnd, WM_ENABLE_CHECKBOXES, 0, 0);
            break;

        case WM_ENABLE_CHECKBOXES: // Custom message to enable checkboxes after login
            if (featureGravity == 1) {
                EnableWindow(chkoptionGravity, TRUE);
            } else {
                EnableWindow(chkoptionGravity, FALSE);
            }

            if (featureZoom == 1) {
                EnableWindow(chkoptionZoom, TRUE);
            } else {
                EnableWindow(chkoptionZoom, FALSE);
            }
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

void SaveLoginCredentials(const std::string& login, const std::string& password) {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\config.txt";

    std::ofstream file(configFilePath);
    if (file.is_open()) {
        file << "login=" << login << std::endl;
        file << "password=" << password << std::endl;
        file.close();
        Log("Login credentials saved successfully");

        // Attempt to login again
        if (Login(login, password)) {
            Log("Login successful after saving credentials");
            // Proceed with enabling features or other necessary steps
            SendMessage(hwnd, WM_ENABLE_CHECKBOXES, 0, 0);
        } else {
            Log("Login failed after saving credentials");
            // open login window again
            CreateLoginWindow(hInstanceGlobal);
        }
    } else {
        Log("Failed to open config file for writing");
    }
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
        file << "optionGravity=" << optionGravity << std::endl;
        file << "optionMoonjump=" << optionMoonjump << std::endl;
        file << "optionZoom=" << optionZoom << std::endl;
        file.close();
        Log("Settings saved successfully");
    } else {
        Log("Failed to open settings file for writing");
    }
}

// Add global variables for login credentials
std::string login;
std::string password;

void LoadLoginCredentials(HINSTANCE hInstance) {
    std::string configFilePath = std::string(appDataPath) + "\\Sylent-X\\config.txt";

    std::ifstream file(configFilePath);
    bool loginFound = false;
    bool passwordFound = false;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("login=") != std::string::npos) {
                login = line.substr(line.find("=") + 1);
                loginFound = true;
            }
            if (line.find("password=") != std::string::npos) {
                password = line.substr(line.find("=") + 1);
                passwordFound = true;
            }
        }
        file.close();
        Log("Login credentials loaded successfully");
    } else {
        Log("Failed to open config file for reading");
    }

    if (!loginFound || !passwordFound) {
        Log("Login or password not found in config file. Opening login window.");
        CreateLoginWindow(hInstance);
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
            if (line.find("optionGravity=") != std::string::npos)
                optionGravity = (line.substr(line.find("=") + 1) == "1");
            if (line.find("optionMoonjump=") != std::string::npos)
                optionMoonjump = (line.substr(line.find("=") + 1) == "1");
            if (line.find("optionZoom=") != std::string::npos)
                optionZoom = (line.substr(line.find("=") + 1) == "1");
        }
        file.close();
        Log("Settings loaded successfully");
    } else {
        LogDebug("Settings file not found");
    }

    // Load login credentials
    LoadLoginCredentials(hInstanceGlobal);
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
        // LogDebug("Process snapshot created for process ID: " + std::to_string(procId));
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                wchar_t wModuleName[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, modEntry.szModule, -1, wModuleName, MAX_PATH);
                // LogDebug("Checking module: " + std::string(modEntry.szModule));
                if (!_wcsicmp(wModuleName, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    // LogDebug("Module found: " + std::string(modEntry.szModule) + 
                    //     " at address: " + std::to_string(modBaseAddr) + 
                    //     " in process ID: " + std::to_string(procId));
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

void MemoryManipulation(const std::string& option) {
    // LogDebug("Performing memory manipulation for " + option);

    // Get process ID for ROClientGame.exe
    pid = GetProcessIdByName(L"ROClientGame.exe");
    if (pid == 0) {
        Log("Failed to find ROClientGame.exe process");
        MessageBox(NULL, "Failed to find ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }
    // LogDebug("Process ID for ROClientGame.exe: " + std::to_string(pid));

    // Open the process with full access
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        Log("Failed to open ROClientGame.exe process. Error code: " + std::to_string(GetLastError()));
        MessageBox(NULL, "Failed to open ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }
    // LogDebug("Successfully opened process with ID: " + std::to_string(pid));

    // Get the base address of ROClientGame.exe
    uintptr_t baseAddress = GetModuleBaseAddress(pid, L"ROClientGame.exe");
    if (baseAddress == 0) {
        Log("Failed to get the base address of ROClientGame.exe");
        CloseHandle(hProcess);
        return;
    }
    // LogDebug("Base address of ROClientGame.exe: " + std::to_string(baseAddress));

    // Use the fetched pointers
    for (const auto& pointer : pointers) {
        if (pointer.name == option) {
            uintptr_t optionPointer = baseAddress + pointer.address;
            // LogDebug(option + " pointer address: " + std::to_string(optionPointer));

            uintptr_t finalAddress = optionPointer;
            SIZE_T bytesRead;

            // Apply each offset sequentially
            for (size_t i = 0; i < pointer.offsets.size(); ++i) {
                if (ReadProcessMemory(hProcess, (LPCVOID)finalAddress, &finalAddress, sizeof(finalAddress), &bytesRead)) {
                    if (bytesRead != sizeof(finalAddress)) {
                        LogDebug("Failed to read the " + option + " pointer address. Bytes read: " + std::to_string(bytesRead));
                        return;
                    }
                    finalAddress += pointer.offsets[i];
                } else {
                    LogDebug("Failed to read " + option + " pointer from memory. Error code: " + std::to_string(GetLastError()));
                    return;
                }
            }

            // Convert finalAddress to hexadecimal string
            std::stringstream ss;
            ss << std::hex << std::uppercase << finalAddress;
            std::string finalAddressHex = ss.str();

            // Log the final address and value being written
            // LogDebug("Final address: " + finalAddressHex);

            float newValue = 0.0f;

            if (option == "zoom") {
                newValue = optionZoom ? 25.0f : 15.0f;
            } else if (option == "moonjump") {
                newValue = optionMoonjump ? 1.0f : 4.0f;
            } else if (option == "gravity") {
                newValue = optionGravity ? -8.0f : 8.0f;
            } else if (option == "gravitydown") {
                newValue = 8.0f;
            }

            // LogDebug("Writing value: " + std::to_string(newValue) + " to address: " + finalAddressHex);

            if (WriteProcessMemory(hProcess, (LPVOID)finalAddress, &newValue, sizeof(newValue), NULL)) {
                // LogDebug("Successfully wrote new " + option + " value: " + std::to_string(newValue));
            } else {
                LogDebug("Failed to write new " + option + " value. Error code: " + std::to_string(GetLastError()));
            }
        }
    }

    // Close the process handle
    CloseHandle(hProcess);
    // LogDebug("Memory manipulation completed");
}