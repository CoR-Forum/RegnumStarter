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

// Define a static variable to hold the selected text color
static ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white color
static bool enableRainbow = false;
static float rainbowSpeed = 0.1f;
// Declare chatInput as a static variable
static char chatInput[128] = "";

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



void UpdateRainbowColor(float speed) {
    float time = ImGui::GetTime() * speed;
    textColor.x = (sin(time) * 0.5f) + 0.5f;
    textColor.y = (sin(time + 2.0f) * 0.5f) + 0.5f;
    textColor.z = (sin(time + 4.0f) * 0.5f) + 0.5f;
}

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


void RenderUI()
{
    static ImVec4 disabledTextColor = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
    // Show the color wheel to allow the user to change the disabled text color
    ImGui::ShowColorWheel(disabledTextColor);

    // Render the InputTextWithHint with the updated disabled text color
    static char chatInput[256] = "";
    ImGui::InputTextWithHint("##ChatInput", "Type your message here...", chatInput, IM_ARRAYSIZE(chatInput));
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
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.098f, 0.098f, 0.902f, 1.0f)); // Color #1919e6
    ImGui::TextDisabled("(?)");
    ImGui::PopStyleColor();
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
    bool show_admin_window = false;
    bool show_settings_window = false;
    bool show_info_window = false;
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
            ImGui::Begin("Login", &show_login_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

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

            ImGui::End();
        }

        if (show_register_window) {
            ImGui::Begin("Register", &show_register_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

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

            if (ImGui::Button("Exit")) {
                done = true;
            }

            ImGui::End();
        }

        if (show_forgot_password_window) {
            ImGui::Begin("Forgot Password", &show_forgot_password_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

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

            ImGui::End();
        }

        if (show_token_window) {
            ImGui::Begin("Enter Token and New Password", &show_token_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            static char token[128] = "";
            static char newPassword[128] = "";

            ImGui::InputText("Token", token, IM_ARRAYSIZE(token));
            ImGui::SameLine();
            ShowHelpMarker("Sent to you by e-mail");

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

            ImGui::End();
        }

        if (show_info_window) {
            ImGui::Begin("Credits", &show_info_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("Sylent-X %s", currentVersion.c_str());
            ImGui::Text("Made with hate in Germany by Francis, Shaiko and Manu.");
            ImGui::Text("Special thanks to the Champions of Regnum community for their support and feedback.");
            ImGui::End();
        }

        if (show_settings_window) {
            ImGui::Begin("Settings", &show_settings_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            if (enableRainbow) {
                UpdateRainbowColor(rainbowSpeed);
            }
            // Dropdown for selecting the update channel
            static int updateChannel = 0;
            const char* updateChannels[] = { "Stable", "Beta", "Dev" };
            

            // Checkbox to enable/disable rainbow effect
            ImGui::Checkbox("Enable Rainbow Text", &enableRainbow);

            // Slider to control the speed of the rainbow effect
            ImGui::SliderFloat("Rainbow Speed", &rainbowSpeed, 0.01f, 1.0f, "%.2f");
            ImGui::Combo("Update Channel", &updateChannel, updateChannels, IM_ARRAYSIZE(updateChannels));   

            // Show the color wheel
            ImGui::ShowColorWheel(textColor);  
               

            if (ImGui::Button("Save Settings")) {
                SaveSettings();
            }

            ImGui::End();
        }


        if (show_main_window) {
            std::string windowTitle = "Sylent-X " + currentVersion;
            static bool mainWindowIsOpen = true; // Add a boolean to control the window's open state
            ImGui::Begin(windowTitle.c_str(), &mainWindowIsOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

            // close the window if the user clicks the close button
            if (!mainWindowIsOpen) {
                PostQuitMessage(0);
            }

            static bool optionGravity = false;
            static bool optionZoom = false;
            static bool optionMoonjump = false;

            if (ImGui::CollapsingHeader("View", ImGuiTreeNodeFlags_DefaultOpen)) {
                static float zoomValue = 15.0f; // Default zoom value
                static bool prevZoomState = false; // Track previous state of the checkbox

                ImGui::Checkbox("Enable Zoom", &optionZoom);
                ImGui::SameLine();

                if (optionZoom) {
                    if (ImGui::SliderFloat("Zoom", &zoomValue, 15.0f, 60.0f)) { // Adjust the range as needed
                        MemoryManipulation("zoom", zoomValue);
                    }
                } else if (prevZoomState) {
                    // Reset zoom value to 15.0f when checkbox is unchecked
                    zoomValue = 15.0f;
                    MemoryManipulation("zoom", zoomValue);
                }

                prevZoomState = optionZoom; // Update previous state
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::BeginDisabled(!featureGravity);
                if (ImGui::Checkbox("Gravity", &optionGravity)) {
                    float newValue = optionGravity ? -8.0f : 8.0f;
                    MemoryManipulation("gravity", newValue);
                }
                ImGui::EndDisabled();
                if (!featureGravity) {
                    ImGui::SameLine();
                    ShowHelpMarker("This feature is not available in your current license.");
                }
                ImGui::BeginDisabled(!featureMoonjump);
                if (ImGui::Checkbox("Moonjump", &optionMoonjump)) {
                    float newValue = optionMoonjump ? 1.0f : 4.0f;
                    MemoryManipulation("moonjump", newValue);
                }
                ImGui::EndDisabled();
                if (!featureMoonjump) {
                    ImGui::SameLine();
                    ShowHelpMarker("This feature is not available in your current license.");
                }

                ImGui::BeginDisabled(!featureMoonwalk);
                if (ImGui::Checkbox("Moonwalk", &optionMoonwalk)) {
                    float newValue = optionMoonwalk ? 9.219422856E-41f : 0.0f;
                    MemoryManipulation("moonwalk", newValue);
                }
                ImGui::EndDisabled();
                if (!featureMoonwalk) {
                    ImGui::SameLine();
                    ShowHelpMarker("This feature is not available in your current license.");
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SameLine();
            if (ImGui::Button("Chat")) {
                show_chat_window = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Settings")) {
                show_settings_window = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Credits")) {
                show_info_window = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Feedback")) {
                show_feedback_window = true;
            }

            if (isAdmin) {
                ImGui::SameLine();
                if (ImGui::Button("Admin")) {
                    GetAllUsers();
                    show_admin_window = true; // Show the admin window
                }

                ImGui::SameLine();
                ImGui::Checkbox("Debug", &debugLog);
            }

            ImGui::SameLine();
            if (ImGui::Button("Logout")) {
                Logout(); // Use the logic from ApiHandler.cpp
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

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
            ImGui::InputTextWithHint("##ChatInput", "Type your message...", chatInput, IM_ARRAYSIZE(chatInput));
            ImGui::PopItemWidth(); // Reset the item width to default
            ImGui::SameLine();
            if (ImGui::Button("Send")) {
                if (strlen(chatInput) > 0) {
                    SendChatMessage(chatInput);
                    chatInput[0] = '\0'; // Clear input field
                }
            }

            ImGui::End();
        }

        if (show_feedback_window) {
            ImGui::Begin("Feedback", &show_feedback_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

            static int feedbackType = 0;
            const char* feedbackTypes[] = { "Suggestion", "Bug Report", "Other" };

            ImGui::Combo("Type", &feedbackType, feedbackTypes, IM_ARRAYSIZE(feedbackTypes));
            ImGui::InputTextMultiline("Feedback", feedbackText, IM_ARRAYSIZE(feedbackText), ImVec2(480, ImGui::GetTextLineHeight() * 10));

            if (ImGui::Button("Submit")) {
                // Handle feedback submission logic here
                SendFeedbackToDiscord(feedbackText, feedbackTypes[feedbackType]); // Pass feedback text and type
            }

            ImGui::End();
        }

        if (show_chat_window) {
            ImGui::Begin("Chat", &show_chat_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            // Log display box at the bottom
            ImGui::BeginChild("ChatMessages", ImVec2(550, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            for (const auto& msg : g_chatMessages) {
                ImGui::TextWrapped("%s", msg.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f); // Scroll to the bottom
            }

            ImGui::EndChild();

            ImGui::InputTextWithHint("##ChatInput", "Type your message here...", chatInput, IM_ARRAYSIZE(chatInput));

            ImGui::SameLine();
            
            if (ImGui::Button("Send Message")) {
                if (strlen(chatInput) > 0) {
                    SendChatMessage(chatInput);
                    chatInput[0] = '\0'; // Clear input field
                }
            }

            ImGui::End();
        }

        if (show_admin_window) { // Add this block
            ImGui::Begin("Admin Panel", &show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("All Users");

            DisplayUsersTable();
                        
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

void DisplayUsersTable() {
    // Parse the JSON data
    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(GetAllUsersRawJson);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    // Check if jsonData contains the "users" array
    if (!jsonData.contains("users") || !jsonData["users"].is_array()) {
        std::cerr << "Expected JSON array 'users' but got: " << jsonData.type_name() << std::endl;
        return;
    }

    // Begin the ImGui table with a maximum height
    ImGui::BeginChild("UsersTableChild", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::BeginTable("AllUsersTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 20.0f);
        ImGui::TableSetupColumn("Username", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Email", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Role", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableHeadersRow();

        // Iterate over the user data and populate the table rows
        for (const auto& user : jsonData["users"]) {
            if (!user.is_object()) {
                std::cerr << "Expected JSON object but got: " << user.type_name() << std::endl;
                continue;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", user.value("id", 0));
            ImGui::TableNextColumn();
            ImGui::Text("%s", user.value("username", "N/A").c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", user.value("email", "N/A").c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", user.value("is_admin", 0) ? "Admin" : "User");
            ImGui::TableNextColumn();
            // if is_active, show disable button, else show enable button
            if (user.value("is_active", 0)) {
                if (ImGui::Button(("Enabled##" + std::to_string(user.value("id", 0))).c_str())) {
                    // Placeholder for disable logic
                    std::cout << "User " << user.value("username", "N/A") << " disabled." << std::endl;
                }
            } else {
                if (ImGui::Button(("Disabled##" + std::to_string(user.value("id", 0))).c_str())) {
                    // Placeholder for enable logic
                    std::cout << "User " << user.value("username", "N/A") << " enabled." << std::endl;
                }
            }
            ImGui::TableNextColumn();
            // button to call ToggleUserBan with user["id"]
            if (user.value("is_banned", 0)) {
                if (ImGui::Button(("Unban##" + std::to_string(user.value("id", 0))).c_str())) {
                    ToggleUserBan(user.value("id", 0));
                }
            } else {
                if (ImGui::Button(("Ban##" + std::to_string(user.value("id", 0))).c_str())) {
                    ToggleUserBan(user.value("id", 0));
                }
            }
        }
    
    ImGui::EndTable();
    ImGui::EndChild();
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

void ContinuousMemoryWrite(const std::string& option) {
    while (isWriting) {
        MemoryManipulation(option, 0.0f); // Adjust the value as needed
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust the interval as needed
    }
}
