#include <windows.h>
#include <urlmon.h>
#include <comdef.h>
#include <objbase.h>
#include <tlhelp32.h>
#include <wininet.h>
#include "Utils.h"
#include "Updater.cpp"
#include "Logger.cpp"
#include "ApiHandler.cpp"
#include "Windows.cpp"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")

#define WM_CLOSE_REGISTRATION_WINDOW (WM_USER + 1)

std::atomic<bool> isWriting(false);
std::thread memoryThread;

void ContinuousMemoryWrite(const std::string& option) {
    while (isWriting) {
        MemoryManipulation(option);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust the interval as needed
    }
}

// Constants
const std::string currentVersion = "0.1.50"; // Current version of the application
const char* appName = "Sylent-X";

HANDLE hProcess = nullptr; // Handle to the target process (ROClientGame.exe)
DWORD pid; // Process ID of the target process

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInstanceGlobal = hInstance; // Assign to global variable

    Log("Sylent-X " + currentVersion + ". Made with hate in Germany.");

    LoadSettings();

    // Register and create the main window
    if (!RegisterMainWindowClass(hInstance, appName)) {
        MessageBox(NULL, "Failed to register window class.", "Error", MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateMainWindow(hInstance, appName, currentVersion, nCmdShow);
    if (hwnd == NULL) {
        MessageBox(NULL, "Failed to create window.", "Error", MB_ICONERROR);
        return 0;
    }

    // Set the global keyboard hook
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    if (!hKeyboardHook) {
        MessageBox(NULL, "Failed to set keyboard hook.", "Error", MB_ICONERROR);
        return 0;
    }

    if (!Login(login, password)) {
        Log("Login failed.");
        OpenLoginWindow();
    }

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