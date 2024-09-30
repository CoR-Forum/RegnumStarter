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
#include "Updater.cpp"
#include "Utils.h"

#pragma comment(lib, "urlmon.lib")

// Define GUIDs for IID_IBindStatusCallback and IID_IUnknown
const IID IID_IBindStatusCallback = {0x79eac9c1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}};
const IID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

// Constants
const char* appDataPath = getenv("APPDATA");
const char* appName = "Sylent-X";
const UINT WM_START_SELF_UPDATE = WM_USER + 1; // Custom message identifier
const std::string currentVersion = "0.1.3"; // Current version of the application

// Checkboxes states
bool optionNoclip = false;
bool optionSpeedhack = false;

// Handle to the target process (ROClientGame.exe)
HANDLE hProcess = nullptr;

// Handle to the log display control
HWND hLogDisplay = nullptr;

// Deque to store the last 50 log messages
std::deque<std::string> logMessages;

// Handle to the background image
HBITMAP hBackgroundImage = nullptr;

// Function Prototypes
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void SaveSettings();
void LoadSettings();
void MemoryManipulation();
void UpdateLogDisplay();
void Log(const std::string& message);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Log("Sylent-X " + currentVersion + " started");
    Log("🇩🇪 Made with hate in Germany 🇩🇪");

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

    // Create the window
    HWND hwnd = CreateWindowEx(0, appName, "Sylent-X", WS_OVERLAPPEDWINDOW,
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
    static HWND chkoptionNoclip, chkoptionSpeedhack;

    switch (msg) {
        case WM_CREATE:
            Log("Creating checkboxes");

            // Load the background image
            hBackgroundImage = (HBITMAP)LoadImage(NULL, "background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            if (!hBackgroundImage) {
                Log("Failed to load background image");
            }

            // Create checkboxes
            chkoptionNoclip = CreateWindow("BUTTON", "Enable Noclip", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 50, 150, 20, hwnd, (HMENU)1, NULL, NULL);
            chkoptionSpeedhack = CreateWindow("BUTTON", "Enable Speedhack", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                      20, 80, 150, 20, hwnd, (HMENU)2, NULL, NULL);
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
                MemoryManipulation();  // Trigger memory change when option changes
            }
            if (LOWORD(wParam) == 2) {
                optionSpeedhack = !optionSpeedhack;
                SendMessage(chkoptionSpeedhack, BM_SETCHECK, optionSpeedhack ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Speedhack toggled");
                MemoryManipulation();  // Trigger memory change when option changes
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

void MemoryManipulation() {
    Log("Performing memory manipulation");
    DWORD processID;
    HWND hwnd = FindWindow(NULL, "ROClientGame");
    if (!hwnd) {
        Log("ROClientGame.exe not found");
        MessageBox(NULL, "ROClientGame.exe not found.", "Error", MB_ICONERROR);
        return;
    }

    GetWindowThreadProcessId(hwnd, &processID);
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        Log("Failed to open ROClientGame.exe process");
        MessageBox(NULL, "Failed to open ROClientGame.exe process.", "Error", MB_ICONERROR);
        return;
    }

    // Example memory manipulation (addresses would be different in actual game)
    int newValue = optionNoclip ? 1 : 0;
    WriteProcessMemory(hProcess, (LPVOID)0x00AABBCC, &newValue, sizeof(newValue), NULL);

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
    if (logMessages.size() > 50) {
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