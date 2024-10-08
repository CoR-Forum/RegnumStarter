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
#include "Keyboard.cpp"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include "Style.cpp"
#include "ApiHandler.cpp"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "dwmapi.lib")

// Discord webhook URL
const std::string webhook_url = "https://discord.com/api/webhooks/1289932329778679890/Erl7M4hc12KnajYOqeK9jGOpE_G53qonvUcXHIuGb-XvfuA_VkTfI_FF3p1PROFXkL_6";

#define WM_CLOSE_REGISTRATION_WINDOW (WM_USER + 1)

static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
static char feedbackSender[128] = ""; // Add this line

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
bool show_login_window = true;
bool show_main_window = false;
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern bool featureZoom;
extern bool featureGravity;
extern bool featureMoonjump;
extern bool featureMoonwalk;
extern std::string login;

std::vector<Pointer> pointers;


void SendFeedbackToDiscord(const std::string& feedback, const std::string& feedbackType) {
    HINTERNET hSession = InternetOpenA("FeedbackSender", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession) {
        std::cerr << "InternetOpenA failed" << std::endl;
        return;
    }

    HINTERNET hConnect = InternetConnectA(hSession, "discord.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        std::cerr << "InternetConnectA failed" << std::endl;
        InternetCloseHandle(hSession);
        return;
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/api/webhooks/1289932329778679890/Erl7M4hc12KnajYOqeK9jGOpE_G53qonvUcXHIuGb-XvfuA_VkTfI_FF3p1PROFXkL_6", NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        std::cerr << "HttpOpenRequestA failed" << std::endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return;
    }

    std::string headers = "Content-Type: application/json\r\n";
    std::string payload = "{\"content\": \"Feedback from: " + login + " Type: " + feedbackType + " Input: " + feedback + "\"}";

    BOOL result = HttpSendRequestA(hRequest, headers.c_str(), headers.length(), (LPVOID)payload.c_str(), payload.length());
    if (!result) {
        std::cerr << "HttpSendRequestA failed" << std::endl;
    } else {
        std::cout << "Feedback submitted successfully." << std::endl;
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
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


// Declare the Register function
void RegisterUser(const std::string& username, const std::string& email, const std::string& password);

void ShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Log("Sylent-X " + currentVersion + ". Made with hate in Germany.");
    // Create a named mutex
    HANDLE hMutex = CreateMutex(NULL, TRUE, _T("Sylent-X-Mutex"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, _T("Sylent-X is already running."), _T("Error"), MB_ICONERROR | MB_OK);
        return 1;
    }

    LoadSettings();
    SelfUpdate();

    bool loginSuccess = Login(login, password);
    if (loginSuccess) {
        Log("Auto-login successful");
        show_login_window = false;
        show_main_window = true;
        // InitializePointers(); // Initialize pointers after successful login
    } else {
        Log("Auto-login failed");
        show_login_window = true;
    }

    // Register and create the main window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Sylent-X", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, _T("Sylent-X"), NULL, WS_POPUP | WS_VISIBLE, 0, 0, 1200, 1000, NULL, NULL, wc.hInstance, NULL);
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
    static char feedbackText[1024] = "";
    static char chatInput[256] = "";
    static char forgotPasswordEmail[128] = "";
    static std::vector<std::string> chatMessages;

    bool show_register_window = false;
    bool show_feedback_window = false;
    bool show_chat_window = false;
    bool show_forgot_password_window = false;
    bool show_token_window = false;
    bool show_admin_window = false; // Add this line
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
                    show_main_window = true;
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

            if (ImGui::Button("Forgot Password")) {
                show_login_window = false;
                show_forgot_password_window = true;
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

        if (show_forgot_password_window) {
            ImGui::Begin("Forgot Password");
            ImGui::SetWindowSize(ImVec2(500, 200));

            ImGui::InputText("Email", forgotPasswordEmail, IM_ARRAYSIZE(forgotPasswordEmail));

            if (ImGui::Button("Submit")) {
                if (ResetPasswordRequest(forgotPasswordEmail)) {
                    show_forgot_password_window = false;
                    show_token_window = true;
                } else {
                    ImGui::Text("Failed to send reset password request. Please try again.");
                }
            }

            if (ImGui::Button("I already have a token")) {
                show_forgot_password_window = false;
                show_token_window = true;
            }

            if (ImGui::Button("Back to Login")) {
                show_forgot_password_window = false;
                show_login_window = true;
            }

            if (ImGui::Button("Close Application")) {
                done = true;
            }

            ImGui::End();
        }

        if (show_token_window) {
            ImGui::Begin("Enter Token and New Password");
            ImGui::SetWindowSize(ImVec2(500, 300));

            static char token[128] = "";
            static char newPassword[128] = "";

            ImGui::InputText("Token", token, IM_ARRAYSIZE(token));
            ImGui::InputText("New Password", newPassword, IM_ARRAYSIZE(newPassword), ImGuiInputTextFlags_Password);

            static std::string statusText = "";

            if (ImGui::Button("Submit")) {
                // Implement the logic to verify the token and update the password
                if (SetNewPassword(token, newPassword)) {
                    MessageBox(NULL, "Password updated successfully. You may now login.", "Success", MB_ICONINFORMATION);
                    show_token_window = false;
                    show_login_window = true;
                } else {
                    statusText = "Failed to set new password. Please try again.";
                }
            }
            ImGui::SameLine();
            ImGui::Text("%s", statusText.c_str());

            if (ImGui::Button("Request new token")) {
                show_token_window = false;
                show_forgot_password_window = true;
            }

            if (ImGui::Button("Back to Login")) {
                show_token_window = false;
                show_login_window = true;
            }

            if (ImGui::Button("Close Application")) {
                done = true;
            }

            ImGui::End();
        }

        if (show_main_window) {
            std::string windowTitle = "Welcome, Sylent-X User! - Version " + currentVersion;
            ImGui::Begin(windowTitle.c_str());
            ImGui::SetWindowSize(ImVec2(600, 600));

            static bool optionGravity = false;
            static bool optionZoom = false;
            static bool optionMoonjump = false;

            if (ImGui::CollapsingHeader("POV")) {
                static float zoomValue = 15.0f; // Default zoom value
                ImGui::Checkbox("Enable Zoom", &optionZoom);
                ImGui::SameLine();
                if (optionZoom && ImGui::SliderFloat("Zoom", &zoomValue, 10.0f, 50.0f)) { // Adjust the range as needed
                    MemoryManipulation("zoom", zoomValue);
                }
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Movement")) {
                ImGui::BeginDisabled(!featureGravity);
                if (ImGui::Checkbox("Gravity", &optionGravity)) {
                    float newValue = optionGravity ? -8.0f : 8.0f;
                    // print all global pointers from g_pointers
                    LogDebug("Printing all pointers: ");
                    for (const auto& pointer : g_pointers) {
                        std::stringstream ss;
                        ss << std::hex << pointer.address;
                        LogDebug("Pointer: " + pointer.name + " Address: 0x" + ss.str() + " Offsets: ");
                        for (const auto& offset : pointer.offsets) {
                            ss.str(""); // Clear the stringstream
                            ss << std::hex << offset;
                            LogDebug("Offset: 0x" + ss.str());
                        }
                    }
                    MemoryManipulation("gravity", newValue);
                }
                ImGui::EndDisabled();
                ImGui::BeginDisabled(!featureMoonjump);
                if (ImGui::Checkbox("Moonjump", &optionMoonjump)) {
                    float newValue = optionMoonjump ? 1.0f : 4.0f;
                    MemoryManipulation("moonjump", newValue);
                }
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!featureMoonwalk);
                if (ImGui::Checkbox("Moonwalk", &optionMoonwalk)) {
                    float newValue = optionMoonwalk ? 9.219422856E-41f : 0.0f;
                    MemoryManipulation("moonwalk", newValue);
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

            if (ImGui::Button("Chat")) {
                show_chat_window = true;
            }

            ImGui::SameLine();
            
            if (ImGui::Button("Feedback")) {
                show_feedback_window = true;
                show_main_window = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Logout")) {
                Logout(); // Use the logic from ApiHandler.cpp
            }

            ImGui::SameLine();

            // checkbox to toggle debug logging
            ImGui::Checkbox("Debug Log", &debugLog);

            // button to call admin UI, only visible if isAdmin is true
            if (isAdmin && ImGui::Button("Admin UI")) {
                show_admin_window = true; // Show the admin window
            }

            // Log display box at the bottom
            ImGui::BeginChild("LogMessages", ImVec2(550, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            for (const auto& msg : logMessages) {
                ImGui::TextWrapped("%s", msg.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f); // Scroll to the bottom
            }

            ImGui::EndChild();

            // input field and button to send chat messages using sendChatMessage function
            ImGui::InputTextWithHint("Chat Message", "Type your message here...", chatInput, IM_ARRAYSIZE(chatInput));

            ImGui::SameLine();
            
            if (ImGui::Button("Send Chat")) {
                if (strlen(chatInput) > 0) {
                    SendChatMessage(chatInput);
                    chatInput[0] = '\0'; // Clear input field
                }
            }         

            ImGui::End();
        }

        if (show_feedback_window) {
            ImGui::Begin("Feedback");
            ImGui::SetWindowSize(ImVec2(500, 300));

            static int feedbackType = 0;
            const char* feedbackTypes[] = { "Suggestion", "Bug Report", "Other" };

            ImGui::Combo("Type", &feedbackType, feedbackTypes, IM_ARRAYSIZE(feedbackTypes));
            ImGui::InputTextMultiline("Feedback", feedbackText, IM_ARRAYSIZE(feedbackText), ImVec2(480, ImGui::GetTextLineHeight() * 10));

            if (ImGui::Button("Submit")) {
                // Handle feedback submission logic here
                SendFeedbackToDiscord(feedbackText, feedbackTypes[feedbackType]); // Pass feedback text and type
            }

            if (ImGui::Button("Close")) {
                show_feedback_window = false;
                show_main_window = true;
            }

            ImGui::End();
        }

        if (show_chat_window) {
            ImGui::Begin("Chat");
            ImGui::SetWindowSize(ImVec2(600, 320));

            // Log display box at the bottom
            ImGui::BeginChild("ChatMessages", ImVec2(550, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            for (const auto& msg : g_chatMessages) {
                ImGui::TextWrapped("%s", msg.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f); // Scroll to the bottom
            }

            ImGui::EndChild();

            // input field and button to send chat messages using sendChatMessage function
            ImGui::InputTextWithHint("Chat Message", "Type your message here...", chatInput, IM_ARRAYSIZE(chatInput));

            ImGui::SameLine();
            
            if (ImGui::Button("Send Message")) {
                if (strlen(chatInput) > 0) {
                    SendChatMessage(chatInput);
                    chatInput[0] = '\0'; // Clear input field
                }
            } 

            if (ImGui::Button("Close")) {
                show_chat_window = false;
            }

            ImGui::End();
        }

        if (show_admin_window) { // Add this block
            ImGui::Begin("Admin Panel");
            ImGui::SetWindowSize(ImVec2(600, 400));

            // Add admin-specific controls here
            ImGui::Text("Admin Controls");

            ImGui::Spacing();
            
            ImGui::Text("Hover over the (?) Joshua ;)");
            ImGui::SameLine();
            ShowHelpMarker("Thats a Test for User Joshua");

            ImGui::Spacing();

            // Add UI for generating license keys
            static char licenseKey[128] = "";
            if (ImGui::Button("Generate License Key")) {
                // Logic to generate license key will be implemented later
                strcpy(licenseKey, "GeneratedLicenseKey123"); // Placeholder
            }
            ImGui::InputText("License Key", licenseKey, IM_ARRAYSIZE(licenseKey), ImGuiInputTextFlags_ReadOnly);

            ImGui::Spacing();

            // Add UI for banning users
            static char banUsername[128] = "";
            ImGui::InputText("Username to Ban", banUsername, IM_ARRAYSIZE(banUsername));
            if (ImGui::Button("Ban User")) {
                // Logic to ban user will be implemented later
                // Placeholder for ban logic
                std::cout << "User " << banUsername << " banned." << std::endl;
            }

            ImGui::Spacing();

            if (ImGui::Button("Close Admin Panel")) {
                show_admin_window = false;
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

// Memory class to handle memory operations
class Memory {
public:
    uintptr_t GetBaseAddress(const MemoryAddress& memAddr);
    bool WriteFloat(uintptr_t address, float value);
};

// Function to get the base address of a memory address
uintptr_t Memory::GetBaseAddress(const MemoryAddress& memAddr) {
    LogDebug(L"Getting base address of " + std::wstring(memAddr.name.begin(), memAddr.name.end()) + L" at address: " + std::to_wstring(memAddr.address));
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
        // return;
    }

    // Open the game process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    // Check if the process is valid
    if (!hProcess) {
        LogDebug(L"Failed to open ROClientGame.exe process. Error code: " + std::to_wstring(GetLastError()));
        // return;
    } else {
        LogDebug(L"Successfully opened ROClientGame.exe process: " + std::to_wstring(pid));
    }

    // Get the base address of the game module
    uintptr_t baseAddress = GetModuleBaseAddress(pid, L"ROClientGame.exe");
    // Check if the base address is valid
    if (baseAddress == 0) {
        LogDebug(L"Failed to get the base address of ROClientGame.exe: " + std::to_wstring(GetLastError()));
        CloseHandle(hProcess);
        // return;
    } else {
        LogDebug(L"Base address of ROClientGame.exe: 0x" + std::to_wstring(baseAddress));
    }

    // Initialize the Memory class
    LogDebug(L"Initializing Memory class for " + std::wstring(option.begin(), option.end()) + L" option with new value: " + std::to_wstring(newValue) + L" and base address: " + std::to_wstring(baseAddress) + L" and process ID: " + std::to_wstring(pid));
    LogDebug(L"Global pointers: ");
    for (const auto& pointer : g_pointers) {
        std::stringstream ss;
        ss << std::hex << pointer.address;
        LogDebug("Pointer: " + pointer.name + " Address: 0x" + ss.str() + " Offsets: ");
        for (const auto& offset : pointer.offsets) {
            std::stringstream ss;
            ss << std::hex << offset;
            LogDebug("Offset: 0x" + ss.str());
        }
    }
    // Iterate through all pointers and apply the memory manipulation for those that match the option
    for (const auto& pointer : g_pointers) {
        if (pointer.name == option) {
            LogDebug(L"Found the " + std::wstring(option.begin(), option.end()) + L" pointer at address: " + std::to_wstring(pointer.address) + L" with " + std::to_wstring(pointer.offsets.size()) + L" offsets");

            // Calculate the final address
            uintptr_t optionPointer = baseAddress + pointer.address;
            LogDebug(std::wstring(option.begin(), option.end()) + L" pointer address: " + std::to_wstring(optionPointer));

            // Read the final address
            uintptr_t finalAddress = optionPointer;
            LogDebug(std::wstring(option.begin(), option.end()) + L" final address: " + std::to_wstring(finalAddress));
            SIZE_T bytesRead;

            // Iterate through the offsets
            for (size_t i = 0; i < pointer.offsets.size(); ++i) {
                // Read the final address with the offsets applied
                if (ReadProcessMemory(hProcess, (LPCVOID)finalAddress, &finalAddress, sizeof(finalAddress), &bytesRead)) {
                    // Check if the read was successful
                    if (bytesRead != sizeof(finalAddress)) {
                        LogDebug(L"Failed to read the " + std::wstring(option.begin(), option.end()) + L" pointer address. Error code: " + std::to_wstring(GetLastError()) + L". Got " + std::to_wstring(i) + L" offsets (to be specific: " + std::to_wstring(pointer.offsets[i]) + L")");
                        return;
                    }
                    // Apply the offsets to the final address
                    finalAddress += pointer.offsets[i];
                    LogDebug(L"Got " + std::to_wstring(i) + L" offsets (to be specific: " + std::to_wstring(pointer.offsets[i]) + L"). Final address: " + std::to_wstring(finalAddress));
                } else {
                    LogDebug(L"Failed to read the " + std::wstring(option.begin(), option.end()) + L" pointer address. Error code: " + std::to_wstring(GetLastError()) + L". Got " + std::to_wstring(i) + L" offsets (to be specific: " + std::to_wstring(pointer.offsets[i]) + L")");
                    return;
                }
            }

            // Write the new value to the final address with the offsets applied (if needed)
            if (WriteProcessMemory(hProcess, (LPVOID)finalAddress, &newValue, sizeof(newValue), NULL)) {
                LogDebug(L"Successfully wrote new " + std::wstring(option.begin(), option.end()) + L" value: " + std::to_wstring(newValue));
            } else {
                LogDebug(L"Failed to write new " + std::wstring(option.begin(), option.end()) + L" value. Error code: " + std::to_wstring(GetLastError()).c_str());
            }
        }
    }

    CloseHandle(hProcess);
}

std::atomic<bool> isWriting(false);
std::thread memoryThread;

void ContinuousMemoryWrite(const std::string& option) {
    while (isWriting) {
        MemoryManipulation(option);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust the interval as needed
    }
}