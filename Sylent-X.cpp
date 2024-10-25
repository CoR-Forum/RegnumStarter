#include "includes/Utils.h"
#include "includes/Updater/Updater.cpp"
#include "includes/Logger/Logger.cpp"
#include "includes/API/ApiHandler.cpp"
#include "includes/API/AdminApiHandler.cpp"
#include "Style.cpp"
#include "libs/DirectX/DirectXInit.cpp"
#include "libs/imgui/imgui_impl_dx9.cpp"
#include "libs/imgui/imgui_impl_win32.cpp"
#include "libs/ImageLoader/ImageLoader.cpp"
#include "libs/ImageLoader/FontAwesomeIcons.h"
#include "ui/helper/Markers/HelpMarker.cpp"
#include "ui/helper/Markers/LicenseMarker.cpp"
#include <filesystem> // C++17 or later
#include "includes/process/process.cpp"
#include "includes/chrono/chrono.cpp"
#include "includes/streamproof/streamproof.cpp"
#include "ui/admin/AdminPanel.cpp"
#include "ui/login/pwreset/ForgotPasswordWindow.cpp"
#include "ui/login/pwreset/PasswordResetWindow.cpp"
#include "ui/helper/Rainbow/UpdateRainbowColor.cpp"
#include "ui/login/LoginWindow.cpp"
#include "ui/login/register/RegisterWindow.cpp"
#include "ui/RegnumStarter/RegnumStarter.cpp"
#include "ui/Feedback/FeedbackWindow.cpp"
#include "ui/License/LicenseWindow.cpp"
#include "ui/Movement/MovementWindow.cpp"
#include "ui/Credits/CreditsWindow.cpp"
#include "ui/Player/PlayerWindow.cpp"
#include "ui/View/ViewWindow.cpp"
#include "ui/Chat/ChatWindow.cpp"
#include "ui/WindowStates.h"


#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "dwmapi.lib")

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool g_DeviceLost = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static bool show_license_window = false;
static bool spaceKeyPressed = false;
static bool ctrlKeyPressed = false;
extern bool show_chat_window;
bool fovToggled = false; // Initialize the FOV state

ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

bool g_ShowUI = true;
std::string statusMessage = "";
bool loginSuccess = false;

// Define a variable to store the user-defined hotkey
int userDefinedHotkey = 0;
bool waitingForHotkey = false;

std::vector<Pointer> pointers;

const std::string regnumLoginUser = "username";
const std::string regnumLoginPassword = "password";

// Function to check if a hotkey is pressed
bool IsHotkeyPressed(int hotkey) {
    return GetAsyncKeyState(hotkey) & 0x8000;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Log("Sylent-X " + currentVersion + ". Made with hate in Germany.");
    // Create a named mutex
    HANDLE hMutex = CreateMutex(NULL, TRUE, _T("Sylent-X-Mutex"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, _T("Sylent-X is already running."), _T("Error"), MB_ICONERROR | MB_OK);
        return 1;
    }

    SelfUpdate();
    LoadLoginCredentials(hInstanceGlobal);

    bool loginSuccess = Login(login, password);
    if (loginSuccess) {
        Log("Auto-login successful");
        LoadSettings();
        show_login_window = false;
        show_main_window = true;
    } else {
        Log("Auto-login failed");
        show_login_window = true;
    }

    // Register and create the main window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Sylent-X", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, _T("Sylent-X"), NULL, WS_POPUP | WS_VISIBLE, 0, 0, 820, 580, NULL, NULL, wc.hInstance, NULL);
    SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    LPDIRECT3DTEXTURE9 texture_sylent_icon = nullptr;
    texture_sylent_icon = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_SYLENT_ICON);

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.MouseDrawCursor = false; // Hide ImGui cursor
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ApplyCustomStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);

    static char username[128] = "";
    static char password[128] = "";

    static std::vector<std::string> chatMessages;

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

        // Check for the "Insert" key press
        if (GetAsyncKeyState(VK_INSERT) & 0x8000)
        {
            g_ShowUI = !g_ShowUI;
            // Add a small delay to prevent rapid toggling
            Sleep(200);
        }
        // Check for global key press and release events using Windows API
        if (optionGravity && (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            MemoryManipulation("gravity", -8.0f);
        }

        if (optionGravity && (GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
            MemoryManipulation("gravity", 8.0f);
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        if (g_ShowUI) {
            if (show_login_window) {
                ShowLoginWindow(show_login_window, statusMessage, loginSuccess, show_main_window, textColor);
            }

            if (show_register_window) {
                ShowRegisterWindow(show_register_window, show_login_window, done);
            }

            if (show_forgot_password_window) {
                ShowForgotPasswordWindow(show_forgot_password_window, show_password_reset_window, show_login_window);
            }

            if (show_password_reset_window) {
                ShowPasswordResetWindow(show_password_reset_window, show_login_window, show_forgot_password_window);
            } 

            if (show_main_window) {
                std::string windowTitle = "Sylent-X " + currentVersion;
                static bool mainWindowIsOpen = true; // Add a boolean to control the window's open state
                ImGui::SetNextWindowSize(ImVec2(770, 450), ImGuiCond_FirstUseEver);
                ImGui::Begin(windowTitle.c_str(), &mainWindowIsOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

                ImGui::GetStyle().Colors[ImGuiCol_Text] = textColor;
                ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = textColor;

                if (setting_enableRainbow) {
                    UpdateRainbowColor(setting_rainbowSpeed);
                }

                // close the window if the user clicks the close button
                if (!mainWindowIsOpen) {
                    SaveSettings();
                    PostQuitMessage(0);
                }

                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 200);
                ImGui::Text("Status: %s", sylentx_status.c_str());
                ImGui::SameLine();
                ImGui::Text("Magnat: %d", magnatCurrency);
                
                // Create a child window for the texture
                ImGui::BeginChild("TextureChild", ImVec2(130, 80), true);
                if (texture_sylent_icon) {
                    // Calculate the available space
                    ImVec2 availableSpace = ImGui::GetContentRegionAvail();
                    ImVec2 imageSize = ImVec2(70, 60); // Adjust the size as needed

                    // Calculate the padding to center the image
                    ImVec2 padding = ImVec2((availableSpace.x - imageSize.x) * 0.5f, (availableSpace.y - imageSize.y) * 0.5f);

                    // Add padding
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding.x);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);

                    // Draw the image
                    ImGui::Image((void*)texture_sylent_icon, imageSize);
                } else {
                    ImGui::Text("Texture is null");
                }
                ImGui::EndChild();

                ImGui::SameLine();
                // Create a child window for the texture
                ImGui::BeginChild("Menu", ImVec2(615, 80), true);
                float buttonWidth = 150.0f; // Assuming each button has a width of 150
                float buttonHeight = 30.0f; // Assuming each button has a height of 40
                float spacing = ImGui::GetStyle().ItemSpacing.x; // Get the default spacing between items

                // Calculate total width of all buttons and spacing
                float totalWidth = 3 * buttonWidth + 6 * spacing;

                // Calculate starting X position to center buttons horizontally
                float startX = (615 - totalWidth) / 2.0f;

                // Calculate starting Y position to center buttons vertically
                float startY = (80 - buttonHeight) / 2.0f;

                ImGui::SetCursorPosX(startX);
                ImGui::SetCursorPosY(startY);
                if (ImGui::Button(ICON_FA_EYE " View", ImVec2(buttonWidth, buttonHeight))) {
                    show_movement_window = false;
                    show_settings_window = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumStarter = false;
                    show_player_window = false;
                    show_view_window = true;
                }

                ImGui::SameLine();
                ImGui::SetCursorPosY(startY);
                if (ImGui::Button(ICON_FA_WHEELCHAIR " Movement", ImVec2(buttonWidth, buttonHeight))) {
                    show_view_window = false;
                    show_settings_window = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumStarter = false;
                    show_player_window = false;
                    show_movement_window = true;
                }

                ImGui::SameLine();
                ImGui::SetCursorPosY(startY);
                if (ImGui::Button(ICON_FA_USER " Player", ImVec2(buttonWidth, buttonHeight))) {
                    show_movement_window = false;
                    show_settings_window = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumStarter = false;
                    show_view_window = false;
                    show_player_window = true;
                }

                ImGui::EndChild();

                // Calculate the size of the largest button
                ImVec2 buttonSize = ImVec2(0, 0);
                const char* buttonLabels[] = {
                    "Sylent-X", "Admin", "Chat", "Settings", "RegnumStarter", 
                    "Feedback", "License", "Info", "Logout"
                };
                for (const char* label : buttonLabels) {
                    ImVec2 size = ImGui::CalcTextSize(label);
                    buttonSize.x = std::max(buttonSize.x, size.x + ImGui::GetStyle().FramePadding.x * 2.0f);
                    buttonSize.y = std::max(buttonSize.y, size.y + ImGui::GetStyle().FramePadding.y * 2.0f);
                }

                // Create a child window for the navigation buttons
                ImGui::BeginChild("Navigation", ImVec2(130, 0), true);

                // Calculate the padding to center the buttons
                float childWidth = ImGui::GetWindowWidth();
                float buttonPadding = (childWidth - buttonSize.x) / 2.0f;

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_HOME " Sylent-X", buttonSize)) {
                    show_settings_window = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumStarter = false;
                    show_view_window = false;
                    show_movement_window = false;
                    show_player_window = false;
                }

                if (isAdmin) {
                    ImGui::SetCursorPosX(buttonPadding);
                    if (ImGui::Button(ICON_FA_USER_REGULAR " Admin", buttonSize)) {
                        GetAllUsers();
                        GetAllLicenses();
                        show_admin_window = true; // Show the admin window
                    }

                    ShowAdminPanel(&show_admin_window);
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_COMMENTS " Chat", buttonSize)) {
                    show_chat_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button("RegnumStarter", buttonSize)) {
                    show_settings_window = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_view_window = false;
                    show_movement_window = false;
                    show_player_window = false;
                    LoadRegnumAccounts();
                    show_RegnumStarter = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_COMMENT " Feedback", buttonSize)) {
                    show_settings_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumStarter = false;
                    show_view_window = false;
                    show_movement_window = false;
                    show_player_window = false;
                    show_feedback_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_KEY " License", buttonSize)) {
                    show_settings_window = false;
                    show_feedback_window = false;
                    show_info_window = false;
                    show_RegnumStarter = false;
                    show_view_window = false;
                    show_movement_window = false;
                    show_player_window = false;
                    show_license_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_COG" Settings", buttonSize)) {
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumStarter = false;
                    show_view_window = false;
                    show_movement_window = false;
                    show_player_window = false;
                    show_settings_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_CIRCLE_INFO " Info", buttonSize)) {
                    show_settings_window = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_RegnumStarter = false;
                    show_view_window = false;
                    show_movement_window = false;
                    show_player_window = false;
                    show_info_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_RIGHT_FROM_BRACKET " Logout", buttonSize)) {
                    Logout();
                }

                ImGui::EndChild();

                ImGui::SameLine();

                // Main content area
                ImGui::BeginChild("MainContent", ImVec2(0, 0), true);

                if (show_settings_window) {
                    ImGui::Text("Appearance Settings");

                    ImGui::Checkbox("Enable Rainbow Text", &setting_enableRainbow);
                    ImGui::SameLine();
                    ImGui::SliderFloat("Speed", &setting_rainbowSpeed, 0.01f, 1.0f, "%.2f");

                    ImGui::ShowColorWheel(textColor);

                    ImGui::SliderFloat("Font Size", &setting_fontSize, 0.5f, 2.0f);

                    ImGui::Separator();

                    ImGui::Text("Advanced Settings");

                    if (ImGui::Checkbox("Streamproof", &setting_excludeFromCapture)) {
                        SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
                    }

                    ImGui::SameLine();
                    ShowHelpMarker("Exclude the window from screen capture and hide from taskbar");

                    ImGui::Separator();

                    if (ImGui::Button("Save Settings")) {
                        SaveSettings();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Create Ticket")) {
                        ShellExecute(0, 0, "https://discord.gg/6Nq8VfeWPk", 0, 0, SW_SHOW);
                    }

            } else if (show_feedback_window) {

                ShowFeedbackWindow(show_feedback_window);

            } else if (show_license_window) {
                ShowLicenseWindow(show_license_window);

            } else if (show_info_window) {
                ShowCreditsWindow(show_info_window);  

            } else if (show_view_window) {
                ShowViewWindow(show_view_window, optionZoom, optionFov, featureFov, waitingForHotkey, userDefinedHotkey); 
            } else if (show_movement_window) {

                ShowMovementWindow(show_movement_window);
                    
            } else if (show_player_window) {

                ShowPlayerWindow(show_player_window);
      
            } else if (show_RegnumStarter) {
                ShowRegnumStarter(show_RegnumStarter);
            } else {
                    ImGui::GetStyle().Colors[ImGuiCol_Text] = textColor;
                    ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = textColor;

                    // Log and chat display box at the bottom
                    ImGui::BeginChild("LogMessages", ImVec2(550, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                    for (const auto& msg : logMessages) {
                        ImGui::TextWrapped("%s", msg.c_str());
                    }
                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                        ImGui::SetScrollHereY(1.0f); // Scroll to the bottom
                    }
                    ImGui::EndChild();

                    // input field and button to send chat messages using sendChatMessage function
                    ImGui::PushItemWidth(360); // Set the width of the input field
                    if (ImGui::InputTextWithHint("##ChatInput", "Type your message...", chatInput, IM_ARRAYSIZE(chatInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
                        if (strlen(chatInput) > 0) {
                            SendChatMessage(chatInput);
                            chatInput[0] = '\0'; // Clear input field
                        }
                    }
                    ImGui::PopItemWidth(); // Reset the item width to default
                    ImGui::SameLine();
                    if (ImGui::Button("Send") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
                        if (strlen(chatInput) > 0) {
                            SendChatMessage(chatInput);
                            chatInput[0] = '\0'; // Clear input field
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::End();
            }
        }
        // PoV toggle logic
        if (optionFov && userDefinedHotkey != 0 && IsHotkeyPressed(userDefinedHotkey)) {
            fovToggled = !fovToggled; // Toggle the FOV state
            float newValue = fovToggled ? 0.02999999933f : 0.01745329238f; // Set the new FOV value
            MemoryManipulation("fov", newValue); // Apply the new FOV value
            // Add a small delay to prevent rapid toggling
            Sleep(200);
        }
        if (show_chat_window) {
            ShowChatWindow(show_chat_window);
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

    if (texture_sylent_icon) {
        texture_sylent_icon->Release();
        texture_sylent_icon = nullptr;
    }

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    Log("Sylent-X exiting");
    return 0;
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
    case WM_KEYDOWN:
        if (wParam == VK_INSERT)
        {
            g_ShowUI = !g_ShowUI;
            return 0;
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

HANDLE hProcess = nullptr; // Handle to the target process (ROClientGame.exe)
DWORD pid; // Process ID of the target process
// Function to get the base address of a module
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) {
    // Initialize the module base address to 0
    uintptr_t modBaseAddr = 0;
    // Create a snapshot of the target process
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    // Check if the snapshot is valid
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        // Iterate through the modules in the target process
        if (Module32First(hSnap, &modEntry)) {
            do {
                // Convert the module name to wide char
                wchar_t wModuleName[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, modEntry.szModule, -1, wModuleName, MAX_PATH);
                // Check if the module name matches the target module name
                if (!_wcsicmp(wModuleName, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
                // Continue to the next module
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    // Return the base address of the target module
    LogDebug(L"Base address of " + std::wstring(modName) + L": 0x" + std::to_wstring(reinterpret_cast<uintptr_t>(modBaseAddr)));
    return modBaseAddr;
}

// Function to get the process ID by name
DWORD GetProcessIdByName(const std::wstring& processName) {
    // Initialize the process ID to 0
    DWORD processId = 0;
    // Create a snapshot of the running processes
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    // Check if the snapshot is valid
    if (hSnap != INVALID_HANDLE_VALUE) {
        // Initialize the process entry structure
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        // Iterate through the running processes
        if (Process32First(hSnap, &pe32)) {
            do {
                // Convert the process name to wide char
                std::wstring exeFile(pe32.szExeFile, pe32.szExeFile + strlen(pe32.szExeFile));
                // Check if the process name matches the target process name
                if (!_wcsicmp(exeFile.c_str(), processName.c_str())) {
                    processId = pe32.th32ProcessID;
                    break;
                }
                // Continue to the next process
            } while (Process32Next(hSnap, &pe32));
        }
    }
    CloseHandle(hSnap);
    LogDebug(L"Process ID of " + processName + L": " + std::to_wstring(processId));
    return processId;
}

// Define MemoryAddress struct
struct MemoryAddress {
    std::string name;
    uintptr_t address;
    std::vector<unsigned long> offsets;
};

// Memory class to handle memory operations
class Memory {
public:
    uintptr_t GetBaseAddress(const MemoryAddress& memAddr);
    bool WriteFloat(uintptr_t address, float value);
};

// function to get process path
std::wstring GetProcessPath(DWORD pid) {
    std::wstring path;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        wchar_t buffer[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, buffer, &size)) {
            path = buffer;
        }
        CloseHandle(hProcess);
    }
    return path;
}

// Function to get the base address of a memory address
uintptr_t Memory::GetBaseAddress(const MemoryAddress& memAddr) {
    LogDebug(L"Getting base address of " + std::wstring(memAddr.name.begin(), memAddr.name.end()) + L" at address: " + std::to_wstring(memAddr.address));
    LogDebug("ROClientGame.exe path:  " + WStringToString(GetProcessPath(pid)));
    return GetModuleBaseAddress(pid, L"ROClientGame.exe") + memAddr.address;
}

// Function to write a float value to the game process memory
bool Memory::WriteFloat(uintptr_t address, float value) {
    LogDebug(L"Writing " + std::to_wstring(value) + L" to address: " + std::to_wstring(address));
    return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(value), NULL);
}

// Function to manipulate memory values in the game process
void MemoryManipulation(const std::string& option, float newValue) {
    LogDebug("MemoryManipulation called with option: " + option);
    pid = GetProcessIdByName(L"ROClientGame.exe");
    // Check if the game process is running
    if (pid == 0) {
        LogDebug(L"Failed to find ROClientGame.exe process: " + std::to_wstring(GetLastError()));
        return;
    }
    // Open the game process
    HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_VM_READ, FALSE, pid);
    // Check if the process is valid
    if (!hProcess) {
        LogDebug(L"Failed to open ROClientGame.exe process. Error code: " + std::to_wstring(GetLastError()));
        return;
    } else {
        LogDebug(L"Successfully opened ROClientGame.exe process: " + std::to_wstring(pid));
    }
    // Get the base address of the game module
    uintptr_t baseAddress = GetModuleBaseAddress(pid, L"ROClientGame.exe");
    // Check if the base address is valid
    if (baseAddress == 0) {
        LogDebug(L"Failed to get the base address of ROClientGame.exe: " + std::to_wstring(GetLastError()));
        CloseHandle(hProcess);
        return;
    } else {
        LogDebug(L"Base address of ROClientGame.exe: 0x" + std::to_wstring(baseAddress));
    }

    // Find the pointer with the given option
    auto it = std::find_if(g_pointers.begin(), g_pointers.end(), [&option](const Pointer& ptr) {
        return ptr.name == option;
    });

    if (it == g_pointers.end()) {
        LogDebug(L"Pointer not found for option: " + std::wstring(option.begin(), option.end()));
        CloseHandle(hProcess);
        return;
    }

    const Pointer& pointer = *it;
    uintptr_t finalAddress = baseAddress + pointer.address;
    LogDebug(L"Calculated final address: 0x" + std::to_wstring(finalAddress));

    // If there are offsets, apply them
    if (!pointer.offsets.empty()) {
        SIZE_T bytesRead;
        for (size_t i = 0; i < pointer.offsets.size(); ++i) {
            if (ReadProcessMemory(hProcess, (LPCVOID)finalAddress, &finalAddress, sizeof(finalAddress), &bytesRead)) {
                // Check if the read was successful
                if (bytesRead != sizeof(finalAddress)) {
                    LogDebug(L"Failed to read the " + std::wstring(option.begin(), option.end()) + L" pointer address. Error code: " + std::to_wstring(GetLastError()) + L". Got " + std::to_wstring(i) + L" offsets (to be specific: " + std::to_wstring(pointer.offsets[i]) + L")");
                    CloseHandle(hProcess);
                    return;
                }
                // Apply the offsets to the final address
                finalAddress += pointer.offsets[i];
                LogDebug(L"Got " + std::to_wstring(i) + L" offsets (to be specific: " + std::to_wstring(pointer.offsets[i]) + L"). Final address: " + std::to_wstring(finalAddress));
            } else {
                LogDebug(L"Failed to read the " + std::wstring(option.begin(), option.end()) + L" pointer address. Error code: " + std::to_wstring(GetLastError()) + L". Got " + std::to_wstring(i) + L" offsets (to be specific: " + std::to_wstring(pointer.offsets[i]) + L")");
                CloseHandle(hProcess);
                return;
            }
        }
    }

    // Check memory protection before writing
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(hProcess, (LPCVOID)finalAddress, &mbi, sizeof(mbi)) == 0) {
        LogDebug(L"Failed to query memory protection. Error code: " + std::to_wstring(GetLastError()));
        CloseHandle(hProcess);
        return;
    }

    if (!(mbi.Protect & PAGE_READWRITE) && !(mbi.Protect & PAGE_WRITECOPY)) {
        LogDebug(L"Memory region is not writable. Changing protection...");
        DWORD oldProtect;
        if (!VirtualProtectEx(hProcess, (LPVOID)finalAddress, sizeof(newValue), PAGE_READWRITE, &oldProtect)) {
            LogDebug(L"Failed to change memory protection. Error code: " + std::to_wstring(GetLastError()));
            CloseHandle(hProcess);
            return;
        }
    }

    // Write the new value to the final address
    if (WriteProcessMemory(hProcess, (LPVOID)finalAddress, &newValue, sizeof(newValue), NULL)) {
        LogDebug(L"Successfully wrote new " + std::wstring(option.begin(), option.end()) + L" value: " + std::to_wstring(newValue));
    } else {
        LogDebug(L"Failed to write new " + std::wstring(option.begin(), option.end()) + L" value. Error code: " + std::to_wstring(GetLastError()));
    }

    CloseHandle(hProcess);
}

std::atomic<bool> isWriting(false);
std::thread memoryThread;

std::vector<float> ReadMemoryValues(const std::vector<std::string>& options) {
    std::vector<float> values;
    pid = GetProcessIdByName(L"ROClientGame.exe");
    if (pid == 0) {
        LogDebug(L"Failed to find ROClientGame.exe process: " + std::to_wstring(GetLastError()));
        return values;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        LogDebug(L"Failed to open ROClientGame.exe process. Error code: " + std::to_wstring(GetLastError()));
        return values;
    }

    uintptr_t baseAddress = GetModuleBaseAddress(pid, L"ROClientGame.exe");
    if (baseAddress == 0) {
        LogDebug(L"Failed to get the base address of ROClientGame.exe: " + std::to_wstring(GetLastError()));
        CloseHandle(hProcess);
        return values;
    }

    for (const auto& option : options) {
        auto it = std::find_if(g_pointers.begin(), g_pointers.end(), [&option](const Pointer& ptr) {
            return ptr.name == option;
        });
        if (it == g_pointers.end()) {
            LogDebug(L"Pointer not found for option: " + std::wstring(option.begin(), option.end()));
            continue;
        }

        const Pointer& pointer = *it;
        uintptr_t finalAddress = baseAddress + pointer.address;
        LogDebug(L"Base address for " + std::wstring(option.begin(), option.end()) + L": 0x" + std::to_wstring(baseAddress));
        LogDebug(L"Initial final address for " + std::wstring(option.begin(), option.end()) + L": 0x" + std::to_wstring(finalAddress));

        if (!pointer.offsets.empty()) {
            SIZE_T bytesRead;
            for (size_t i = 0; i < pointer.offsets.size(); ++i) {
                if (ReadProcessMemory(hProcess, (LPCVOID)finalAddress, &finalAddress, sizeof(finalAddress), &bytesRead)) {
                    if (bytesRead != sizeof(finalAddress)) {
                        LogDebug(L"Failed to read the " + std::wstring(option.begin(), option.end()) + L" pointer address. Error code: " + std::to_wstring(GetLastError()) + L". Got " + std::to_wstring(i) + L" offsets (to be specific: " + std::to_wstring(pointer.offsets[i]) + L")");
                        break;
                    }
                    finalAddress += pointer.offsets[i];
                    LogDebug(L"Updated final address for " + std::wstring(option.begin(), option.end()) + L" after offset " + std::to_wstring(i) + L": 0x" + std::to_wstring(finalAddress));
                } else {
                    LogDebug(L"Failed to read the " + std::wstring(option.begin(), option.end()) + L" pointer address. Error code: " + std::to_wstring(GetLastError()));
                    break;
                }
            }
        }

        float value = 0.0f;
        if (ReadProcessMemory(hProcess, (LPCVOID)finalAddress, &value, sizeof(value), NULL)) {
            LogDebug(L"Successfully read " + std::wstring(option.begin(), option.end()) + L" value: " + std::to_wstring(value));
            values.push_back(value);
        } else {
            LogDebug(L"Failed to read " + std::wstring(option.begin(), option.end()) + L" value. Error code: " + std::to_wstring(GetLastError()));
        }
    }

    CloseHandle(hProcess);
    return values;
}
