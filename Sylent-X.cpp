#include <windows.h>
#include <urlmon.h>
#include <comdef.h>
#include <objbase.h>
#include <tlhelp32.h>
#include <wininet.h>
#include <tchar.h> // Add this header for TEXT macro
#include "Utils.h"
#include "Updater.cpp"
#include "Logger.cpp"
#include "ApiHandler.cpp"
#include "Windows.cpp"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include "Style.cpp"
#include "ApiHandler.cpp"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "dwmapi.lib")

#define WM_CLOSE_REGISTRATION_WINDOW (WM_USER + 1)

static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
bool show_login_window = true;
bool show_Sylent_window = false;
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern bool featureZoom;
extern bool featureGravity;
extern bool featureMoonjump;

std::atomic<bool> isWriting(false);
std::thread memoryThread;

void ContinuousMemoryWrite(const std::string& option) {
    while (isWriting) {
        MemoryManipulation(option);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust the interval as needed
    }
}

// Function to initialize DirectX
bool InitDirectX(HWND hwnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
        return false;
    }

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, 
                             D3DCREATE_HARDWARE_VERTEXPROCESSING, 
                             &g_d3dpp, &g_pd3dDevice) < 0) {
        return false;
    }

    return true;
}

// Function to reset the Direct3D device
void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL) {
        IM_ASSERT(0);
    }
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Constants
const std::string currentVersion = "0.1.50"; // Current version of the application
const char* appName = "Sylent-X";

HANDLE hProcess = nullptr; // Handle to the target process (ROClientGame.exe)
DWORD pid; // Process ID of the target process

// Declare the Register function
void RegisterUser(const std::string& username, const std::string& email, const std::string& password);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Log("Sylent-X " + currentVersion + ". Made with hate in Germany.");

    LoadSettings();

    bool loginSuccess = Login(login, password);
    if (loginSuccess) {
        Log("Auto-login successful");
        show_login_window = false;
        show_Sylent_window = true;
        InitializePointers(); // Initialize pointers after successful login
    } else {
        Log("Auto-login failed");
        show_login_window = true;
    }

    // Register and create the main window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Sylent-X", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, _T("Sylent-X"), NULL, WS_POPUP | WS_VISIBLE, 0, 0, 2560, 1440, NULL, NULL, wc.hInstance, NULL);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ApplyCustomStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    static char username[128] = "";
    static char password[128] = "";
    static char regUsername[128] = "";
    static char regPassword[128] = "";
    static char regEmail[128] = "";

    bool show_register_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_DeviceLost) {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if (show_login_window) {
            ImGui::Begin("Login");
            ImGui::SetWindowSize(ImVec2(500, 300));

            ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
            ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
            
            if (ImGui::Button("Login")) {
                loginSuccess = Login(username, password);
                if (loginSuccess) {
                    Log("Login successful");
                    SaveLoginCredentials(username, password);
                    show_login_window = false;
                    show_Sylent_window = true;
                    InitializePointers(); // Initialize pointers after successful login
                } else {
                    Log("Login failed");
                }
            }

            if (loginSuccess) {
                ImGui::Text("Login successful");
            } else {
                ImGui::Text("Login failed");
            }

            if (ImGui::Button("Register")) {
                show_login_window = false;
                show_register_window = true;
            }

            if (ImGui::Button("Close Application")) {
                done = true;
            }

            ImGui::End();
        }

        if (show_register_window) {
            ImGui::Begin("Register");
            ImGui::SetWindowSize(ImVec2(500, 300));

            ImGui::InputText("Username", regUsername, IM_ARRAYSIZE(regUsername));
            ImGui::InputText("Password", regPassword, IM_ARRAYSIZE(regPassword), ImGuiInputTextFlags_Password);
            ImGui::InputText("Email", regEmail, IM_ARRAYSIZE(regEmail));

            if (ImGui::Button("Register")) {
                RegisterUser(regUsername, regEmail, regPassword);
                show_register_window = false;
                show_login_window = true;
            }

            if (ImGui::Button("Back to Login")) {
                show_register_window = false;
                show_login_window = true;
            }

            if (ImGui::Button("Close Application")) {
                done = true;
            }

            ImGui::End();
        }

        if (show_Sylent_window) {
            std::string windowTitle = "Welcome, Sylent-X User! - Version " + currentVersion;
            ImGui::Begin(windowTitle.c_str());
            ImGui::SetWindowSize(ImVec2(500, 300));

            static bool optionGravity = false;
            static bool optionZoom = false;
            static bool optionMoonjump = false;

            if (ImGui::CollapsingHeader("POV")) {
                if (ImGui::Checkbox("Zoom", &optionZoom)) {
                    float newValue = optionZoom ? 25.0f : 15.0f;
                    MemoryManipulation("zoom", newValue);
                }
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Movement")) {
                ImGui::BeginDisabled(!featureGravity);
                if (ImGui::Checkbox("Gravity", &optionGravity)) {
                    float newValue = optionGravity ? -8.0f : 8.0f;
                    MemoryManipulation("gravity", newValue);
                }
                ImGui::EndDisabled();
                ImGui::BeginDisabled(!featureMoonjump);
                if (ImGui::Checkbox("Moonjump", &optionMoonjump)) {
                    float newValue = optionMoonjump ? 1.0f : 4.0f;
                    MemoryManipulation("moonjump", newValue);
                }
                ImGui::EndDisabled();
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("ESP")) {
                // Add ESP related checkboxes here
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Player")) {
                // Add Player related checkboxes here
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Teleport")) {
                // Add Teleport related checkboxes here
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("TrollOptions")) {
                // Add TrollOptions related checkboxes here
            }

            ImGui::Spacing();

            if (ImGui::Button("Close Application")) {
                done = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Logout")) {
                Logout(); // Use the logic from ApiHandler.cpp
            }

            ImGui::End();
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 0.0f), (int)(clear_color.y * clear_color.w * 0.0f), (int)(clear_color.z * clear_color.w * 0.0f), (int)(clear_color.w * 0.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    Log("Sylent-X exiting");
    return 0;
}

// Function to get the base address of a module
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                wchar_t wModuleName[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, modEntry.szModule, -1, wModuleName, MAX_PATH);
                if (!_wcsicmp(wModuleName, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
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

void MemoryManipulation(const std::string& option, float newValue) {
    Log("MemoryManipulation called with option: " + option);
    pid = GetProcessIdByName(L"ROClientGame.exe");
    if (pid == 0) {
        Log("Failed to find ROClientGame.exe process");
        MessageBox(NULL, "Failed to find ROClientGame.exe process.", "Error", MB_ICONERROR | MB_TOPMOST);
        return;
    }

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        Log("Failed to open ROClientGame.exe process. Error code: " + std::to_string(GetLastError()));
        MessageBox(NULL, "Failed to open ROClientGame.exe process.", "Error", MB_ICONERROR | MB_TOPMOST);
        return;
    }

    uintptr_t baseAddress = GetModuleBaseAddress(pid, L"ROClientGame.exe");
    if (baseAddress == 0) {
        MessageBox(NULL, "Failed to get the base address of ROClientGame.exe.", "Error", MB_ICONERROR | MB_TOPMOST);
        CloseHandle(hProcess);
        return;
    }

    for (const auto& pointer : pointers) {
        if (pointer.name == option) {
            uintptr_t optionPointer = baseAddress + pointer.address;
            uintptr_t finalAddress = optionPointer;
            SIZE_T bytesRead;

            for (size_t i = 0; i < pointer.offsets.size(); ++i) {
                if (ReadProcessMemory(hProcess, (LPCVOID)finalAddress, &finalAddress, sizeof(finalAddress), &bytesRead)) {
                    if (bytesRead != sizeof(finalAddress)) {
                        MessageBox(NULL, ("Failed to read the " + option + " pointer address. Bytes read: " + std::to_string(bytesRead)).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
                        return;
                    }
                    finalAddress += pointer.offsets[i];
                } else {
                    MessageBox(NULL, ("Failed to read " + option + " pointer from memory. Error code: " + std::to_string(GetLastError())).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
                    return;
                }
            }

            if (WriteProcessMemory(hProcess, (LPVOID)finalAddress, &newValue, sizeof(newValue), NULL)) {
                // LogDebug("Successfully wrote new " + option + " value: " + std::to_string(newValue));
            } else {
                MessageBox(NULL, ("Failed to write new " + option + " value. Error code: " + std::to_string(GetLastError())).c_str(), "Error", MB_ICONERROR | MB_TOPMOST);
            }
        }
    }

    CloseHandle(hProcess);
}
void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd)

{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;          
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}