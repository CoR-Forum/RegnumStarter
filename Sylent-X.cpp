#include "includes/Utils.h"
#include "includes/Updater/Updater.cpp"
#include "includes/Logger/Logger.cpp"
#include "ApiHandler.cpp"
#include "Style.cpp"
#include "ui/admin/AdminPanel.h"
#include "ui/ForgotPasswordWindow.h"
#include "ui/PasswordResetWindow.h"
#include "libs/DirectX/DirectXInit.h"
#include "ui/helper/UpdateRainbowColor.h"
#include "ui/helper/Markers/HelpMarker.h"
#include "ui/helper/Markers/LicenseMarker.h"
#include "includes/streamproof/streamproof.h"
#include "includes/chrono/chrono.h"
#include "includes/process/process.h"
#include "ui/loadingscreen/LoadingScreen.h"
#include "includes/ImageLoader/ImageLoader.h"
#include <filesystem> // C++17 or later
#include "includes/ImageLoader/FontAwesomeIcons.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "dwmapi.lib")


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static bool show_license_window = false;
static char chatInput[256] = ""; // Declare chatInput as a static variable
static bool spaceKeyPressed = false;
static bool ctrlKeyPressed = false;

ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

bool show_login_window = true;
bool show_main_window = false;
bool show_register_window = false;
bool show_feedback_window = false;
bool show_chat_window = false;
bool show_forgot_password_window = false;
bool show_password_reset_window = false;
bool show_admin_window = false;
bool show_settings_content = false; 
bool show_info_window = false;
bool show_Regnumstarter = false;
bool g_ShowUI = true;
bool show_loading_screen = false;
std::string statusMessage = "";
bool loginSuccess = false;
bool show_texture_window = false;
bool show_View_window = false;
bool show_Movement_window = false;
bool show_Player_window = false;
// Define a variable to store the user-defined hotkey
int userDefinedHotkey = 0;
bool waitingForHotkey = false;
bool fovToggled = false; // Track the state of the FOV toggle

LPDIRECT3DTEXTURE9 myTexture = nullptr;

extern bool featureZoom;
extern bool featureFov;
extern bool featureGravity;
extern bool featureMoonjump;
extern bool featureMoonwalk;
extern bool featureFreecam;
extern bool featureFastfly;
extern bool featureSpeedhack;
extern bool featureFakelag;

std::vector<Pointer> pointers;
std::vector<float> ReadMemoryValues(const std::vector<std::string>& options);

const std::string regnumLoginUser = "username";
const std::string regnumLoginPassword = "password";

// Function to get the key name from the virtual key code
std::string GetKeyName(int virtualKey) {
    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    char keyName[128];
    if (GetKeyNameText(scanCode << 16, keyName, sizeof(keyName)) > 0) {
        return std::string(keyName);
    }
    return "Unknown";
}

// Function to check if the hotkey is pressed
bool IsHotkeyPressed(int hotkey) {
    return GetAsyncKeyState(hotkey) & 0x8000;
}

void runRoClientGame(std::string regnumLoginUser, std::string regnumLoginPassword) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::string regnumPath = setting_regnumInstallPath;
    
    std::string path = regnumPath + "\\LiveServer";
    std::string command = path + " " + regnumLoginUser + " " + regnumLoginPassword;
    std::string workingDirectory = path + "\\LiveServer\\ROClientGame.exe";
    
    if (!CreateProcess(path.c_str(), (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, workingDirectory.c_str(), &si, &pi)) {
        Log("Failed to start the Regnum Online client");
    } else {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
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

    SelfUpdate();
    LoadLoginCredentials(hInstanceGlobal);
    LoadSettings();

    bool loginSuccess = Login(login, password);
    if (loginSuccess) {
        Log("Auto-login successful");
        show_login_window = false;
        show_main_window = true;
    } else {
        Log("Auto-login failed");
        show_login_window = true;
    }

    // Register and create the main window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Sylent-X", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, _T("Sylent-X"), NULL, WS_POPUP | WS_VISIBLE, 0, 0, 1200, 1000, NULL, NULL, wc.hInstance, NULL);
    SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }


    myTexture = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_SYLENT);
    if (!myTexture) {
        MessageBox(NULL, "Failed to load texture", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

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
                static bool settingsWindowIsOpen = true;
                ImGui::Begin("Login", &settingsWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
                
                if (!settingsWindowIsOpen) {
                SaveSettings();
                PostQuitMessage(0);
                }

                ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
                ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
                
                if (ImGui::Button("Login")) {
                    show_loading_screen = true;
                    statusMessage = "Logging in...";
                    loginSuccess = false;
                    show_login_window = false; // Hide the login window

                    std::thread loginThread([&]() {
                        for (int i = 0; i <= 100; ++i) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Simulate progress over 3 seconds
                            UpdateProgressBar(i / 100.0f);
                        }

                        loginSuccess = Login(username, password);
                        if (loginSuccess) {
                            Log("Login successful");
                            SaveLoginCredentials(username, password);
                            show_main_window = true;

                            // Reapply color settings after manual login
                            ImGui::GetStyle().Colors[ImGuiCol_Text] = textColor;
                            ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = textColor;
                        } else {
                            Log("Login failed");
                            show_login_window = true; // Show the login window again if login fails
                        }
                        show_loading_screen = false;
                    });

                    loginThread.detach();
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
                static bool registerWindowIsOpen = true;
                ImGui::Begin("Register", &registerWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

                if (!registerWindowIsOpen) {
                    SaveSettings();
                    PostQuitMessage(0);
                }

                static char regUsername[128] = "";
                static char regPassword[128] = "";
                static char regEmail[128] = "";

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
                if (myTexture) {
                    // Calculate the available space
                    ImVec2 availableSpace = ImGui::GetContentRegionAvail();
                    ImVec2 imageSize = ImVec2(70, 60); // Adjust the size as needed

                    // Calculate the padding to center the image
                    ImVec2 padding = ImVec2((availableSpace.x - imageSize.x) * 0.5f, (availableSpace.y - imageSize.y) * 0.5f);

                    // Add padding
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding.x);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);

                    // Draw the image
                    ImGui::Image((void*)myTexture, imageSize);
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
                    show_Movement_window = false;
                    show_settings_content = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_Regnumstarter = false;
                    show_Player_window = false;
                    show_View_window = true;
                }

                ImGui::SameLine();
                ImGui::SetCursorPosY(startY);
                if (ImGui::Button(ICON_FA_WHEELCHAIR " Movement", ImVec2(buttonWidth, buttonHeight))) {
                    show_View_window = false;
                    show_settings_content = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_Regnumstarter = false;
                    show_Player_window = false;
                    show_Movement_window = true;
                }

                ImGui::SameLine();
                ImGui::SetCursorPosY(startY);
                if (ImGui::Button(ICON_FA_USER " Player", ImVec2(buttonWidth, buttonHeight))) {
                    show_Movement_window = false;
                    show_settings_content = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_Regnumstarter = false;
                    show_View_window = false;
                    show_Player_window = true;
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
                    show_settings_content = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_Regnumstarter = false;
                    show_View_window = false;
                    show_Movement_window = false;
                    show_Player_window = false;
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
                    show_settings_content = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_View_window = false;
                    show_Movement_window = false;
                    show_Player_window = false;
                    show_Regnumstarter = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_COMMENT " Feedback", buttonSize)) {
                    show_settings_content = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_Regnumstarter = false;
                    show_View_window = false;
                    show_Movement_window = false;
                    show_Player_window = false;
                    show_feedback_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_KEY " License", buttonSize)) {
                    show_settings_content = false;
                    show_feedback_window = false;
                    show_info_window = false;
                    show_Regnumstarter = false;
                    show_View_window = false;
                    show_Movement_window = false;
                    show_Player_window = false;
                    show_license_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_COG" Settings", buttonSize)) {
                    show_feedback_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_Regnumstarter = false;
                    show_View_window = false;
                    show_Movement_window = false;
                    show_Player_window = false;
                    show_settings_content = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_CIRCLE_INFO " Info", buttonSize)) {
                    show_settings_content = false;
                    show_feedback_window = false;
                    show_license_window = false;
                    show_Regnumstarter = false;
                    show_View_window = false;
                    show_Movement_window = false;
                    show_Player_window = false;
                    show_info_window = true;
                }

                ImGui::SetCursorPosX(buttonPadding);
                if (ImGui::Button(ICON_FA_RIGHT_FROM_BRACKET " Logout", buttonSize)) {
                    Logout(); // Use the logic from ApiHandler.cpp
                }

                ImGui::EndChild();

                ImGui::SameLine();

                // Main content area
                ImGui::BeginChild("MainContent", ImVec2(0, 0), true);

                if (show_settings_content) {
                    // Settings content
                    ImGui::Text("Appearance Settings");

                    ImGui::Checkbox("Enable Rainbow Text", &setting_enableRainbow);
                    ImGui::SameLine();
                    ImGui::SliderFloat("Speed", &setting_rainbowSpeed, 0.01f, 1.0f, "%.2f");

                    // Show the color wheel
                    ImGui::ShowColorWheel(textColor);

                    // Slider to adjust the font size
                    ImGui::SliderFloat("Font Size", &setting_fontSize, 0.5f, 2.0f);

                    ImGui::Separator();

                    ImGui::Text("Advanced Settings");

                    if (ImGui::Checkbox("Streamproof", &setting_excludeFromCapture)) {
                        SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
                    }

                    ImGui::InputInt("Max Log Messages to store", &setting_log_maxMessages);

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

                    static int feedbackType = 0;
                    static bool feedback_includeLogfile = true;
                    const char* feedbackTypes[] = { "Suggestion", "Bug Report", "Other" };
                    static char feedbackText[1024] = "";
                    static std::string feedbackMessage = "";

                    ImGui::Combo("Type", &feedbackType, feedbackTypes, IM_ARRAYSIZE(feedbackTypes));

                    ImGui::SameLine();
                    ImGui::Checkbox("Include Log File", &feedback_includeLogfile);

                    ImGui::InputTextMultiline("Feedback", feedbackText, IM_ARRAYSIZE(feedbackText), ImVec2(480, ImGui::GetTextLineHeight() * 10));


                    if (ImGui::Button("Submit")) {
                        try {
                            SendFeedback(feedbackTypes[feedbackType], feedbackText, feedback_includeLogfile);
                            feedbackMessage = "Feedback sent successfully!";
                            feedbackText[0] = '\0'; // Clear the feedback text
                        } catch (const std::exception& e) {
                            feedbackMessage = "Failed to send feedback: " + std::string(e.what());
                        }
                    }

                    if (!feedbackMessage.empty()) {
                        ImGui::Text("%s", feedbackMessage.c_str());
                    }
            } else if (show_license_window) {
             
                    static char licenseKey[128] = "";

                    // Display the input text field for the license key
                    ImGui::InputText("License Key", licenseKey, IM_ARRAYSIZE(licenseKey));

                    // Display the submit button
                    if (ImGui::Button("Submit")) {
                        try {
                            ActivateLicense(licenseKey);
                            ImGui::Text("License activated successfully!");
                        } catch (const std::exception& e) {
                            Log("Failed to activate license: " + std::string(e.what()));
                            ImGui::Text("Failed to activate license: %s", e.what());
                        }
                    }
                    ImGui::Separator();
                    // License information from license_runtime_end and license_features
                    ImGui::Text("License Expiry: %s", license_runtime_end.c_str());
            } else if (show_info_window) {

                    ImGui::Text("This software is provided as-is without any warranty. Use at your own risk.");
                    ImGui::Text("Made with hate in Germany by AdrianWho, Manu and Francis");
                    ImGui::Text("Special thanks to the Champions of Regnum community for their support and feedback.");
                    ImGui::Text("Big shoutout to Adrian Lastres. You're the best!");
                    
            } else if (show_View_window) {
                static float zoomValue = 15.0f; // Default zoom value
                static bool prevZoomState = false; // Track previous state of the checkbox

                ImGui::Checkbox("Enable Zoom", &optionZoom);
                if (optionZoom) {
                    ImGui::SameLine();
                    if (ImGui::SliderFloat("Zoom", &zoomValue, 15.0f, 60.0f)) { // Adjust the range as needed
                        MemoryManipulation("zoom", zoomValue);
                    }
                } else if (prevZoomState) {
                    // Reset zoom value to 15.0f when checkbox is unchecked
                    zoomValue = 15.0f;
                    MemoryManipulation("zoom", zoomValue);
                }

                prevZoomState = optionZoom; // Update previous state

                // Check if the checkbox is checked
                ImGui::BeginDisabled(!featureFov);
                if (ImGui::Checkbox("Field of View", &optionFov)) {
                    if (!optionFov) {
                        // If the checkbox is unchecked, reset the FOV value
                        MemoryManipulation("fov", 0.01745329238f);
                    }
                }
                ImGui::EndDisabled();
                ImGui::SameLine();

                // Only allow users to set the hotkey if the checkbox is checked
                if (optionFov) {
                    std::string buttonLabel;
                    if (waitingForHotkey) {
                        buttonLabel = "Press any key...";
                    } else if (userDefinedHotkey == 0) {
                        buttonLabel = "Set Hotkey";
                    } else {
                        buttonLabel = "Hotkey: " + GetKeyName(userDefinedHotkey);
                    }

                    if (ImGui::Button(buttonLabel.c_str())) {
                        waitingForHotkey = true;
                    }

                    if (waitingForHotkey) {
                        for (int key = 0x08; key <= 0xFF; key++) {
                            if (GetAsyncKeyState(key) & 0x8000) {
                                userDefinedHotkey = key;
                                waitingForHotkey = false;
                                break;
                            }
                        }
                    }
                }

                if (!featureFov) {
                    ImGui::SameLine();
                    ShowLicenseMarker();
                }  
            } else if (show_Movement_window) {

                ImGui::BeginDisabled(!featureSpeedhack);
                if (ImGui::Checkbox("SpeedHack", &optionSpeedHack)) {
                    float newValue = optionSpeedHack ? 5.6f : 4.8f;
                    MemoryManipulation("speedhack", newValue);
                }
                if (featureSpeedhack) {
                    ImGui::SameLine();
                    ShowHelpMarker("Use at own risk");
                }
                ImGui::EndDisabled();
                if (!featureSpeedhack) {
                    ImGui::SameLine();
                    ShowLicenseMarker();
                }

                ImGui::BeginDisabled(!featureGravity);
                if (ImGui::Checkbox("Flyhack", &optionGravity)) {
                    MemoryManipulation("gravity");
                }

                ImGui::EndDisabled();
                if (!featureGravity) {
                    ImGui::SameLine();
                    ShowLicenseMarker();
                }

                static float moonjumpValue = 4.0f; // Default moonjump value
                static bool prevjumpState = false; // Track previous state of the checkbox
                ImGui::BeginDisabled(!featureMoonjump);
                if (ImGui::Checkbox("Moonjump", &optionMoonjump)) {
                    if (optionMoonjump) {
                        prevjumpState = true;
                    } else if (prevjumpState) {
                        // Reset zoom value to 4.0f when checkbox is unchecked
                        moonjumpValue = 4.0f;
                        MemoryManipulation("moonjump", moonjumpValue);
                        prevjumpState = false;
                    }
                }
                if (optionMoonjump) {
                    ImGui::SameLine();
                    if (ImGui::SliderFloat("##MoonjumpSlider", &moonjumpValue, 0.3f, 4.0f)) { // Adjust the range as needed
                        MemoryManipulation("moonjump", moonjumpValue);
                    }
                    ImGui::SameLine();
                    ShowHelpMarker("We recommend value 1.00");
                }
                ImGui::EndDisabled();
                if (!featureMoonjump) {
                    ImGui::SameLine();
                    ShowLicenseMarker();
                }

                ImGui::BeginDisabled(!featureMoonwalk);
                if (ImGui::Checkbox("Moonwalk", &optionMoonwalk)) {
                    if (optionMoonwalk) {
                        float newValue = 9.219422856E-41f;
                        MemoryManipulation("moonwalk", newValue);
                        MemoryManipulation("moonwalk", newValue);
                        std::thread(UncheckMoonwalkAfterDelay, std::ref(optionMoonwalk)).detach();
                    }
                }

                ImGui::EndDisabled();
                if (!featureMoonwalk) {
                    ImGui::SameLine();
                    ShowLicenseMarker();
                }

                ImGui::BeginDisabled(!featureFakelag);
                if (ImGui::Checkbox("Fakelag", &optionFakelag)) {
                    if (optionFakelag) {
                        float newValue = 0.0f;
                        MemoryManipulation("fakelag", newValue);
                        MemoryManipulation("fakelagg", newValue);
                        std::thread(UncheckFakelagAfterDelay, std::ref(optionFakelag)).detach();
                    }
                }

                ImGui::EndDisabled();
                if (!featureFakelag) {
                    ImGui::SameLine();
                    ShowLicenseMarker();
                }

                if (isAdmin) {
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::Text("Admin Options:");
                        ImGui::Spacing();
                        static float fastflyValue = 250.0f; // Default moonjump value
                        static bool prevflyState = false; // Track previous state of the checkbox
                        ImGui::BeginDisabled(!featureFastfly);
                        if (ImGui::Checkbox("FastFly", &optionFastFly)) {
                            if (optionFastFly) {
                                prevflyState = true;
                            } else if (prevflyState) {
                                // Reset fly value to 4.8f when checkbox is unchecked
                                fastflyValue = 4.8f;
                                MemoryManipulation("fastfly", fastflyValue);
                                prevflyState = false;
                            }
                        }
                        ImGui::EndDisabled();
                        if (optionFastFly) {
                            ImGui::SameLine();
                            if (ImGui::SliderFloat("##FastFlySlider", &fastflyValue, 4.8f, 250.0f)) { // Adjust the range as needed
                                MemoryManipulation("fastfly", fastflyValue);
                            }
                        }
                        ImGui::SameLine();
                        if (!featureFastfly) {
                            ImGui::SameLine();
                            ShowLicenseMarker();
                        }
                    }
                    
            } else if (show_Player_window) {

                if (IsProcessOpen("ROClientGame.exe")) {

                    std::vector<float> values = ReadMemoryValues({"posx", "posy", "posz"});
                    if (values.size() == 3) {
                        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", values[0], values[1], values[2]);
                    } else {
                        ImGui::Text("Failed to read position values.");
                    }
                }
      
            } else if (show_Regnumstarter) {

                    // large info that indiciates that those settings are not working yet
                    ImGui::Text("These settings are not working yet. Please use the Regnum Online client for now.");
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // A table to display the saved Regnum Accounts using the GetRegnumAccounts function
                    ImGui::Columns(4, "RegnumAccounts");
                    ImGui::Separator();
                    ImGui::Text("Username");
                    ImGui::NextColumn();
                    ImGui::Text("Server");
                    ImGui::NextColumn();
                    ImGui::Text("Referrer");
                    ImGui::NextColumn();
                    ImGui::Text("Actions");
                    ImGui::NextColumn();
                    ImGui::Separator();

                    // Declare the static character arrays at the beginning of the function
                    static char regnumId[128] = "";
                    static char regnumUsername[128] = "";
                    static char regnumPassword[128] = "";
                    static char regnumServer[128] = "";
                    static char regnumReferrer[128] = "";

                    // Example server and referrer options
                    ServerOption serverOptions[] = { {"val", "Valhalla"}, {"ra", "Ra"} };
                    ReferrerOption referrerOptions[] = { {"nge", "NGE"}, {"gmg", "Gamigo"}, {"boa", "Boacompra"} };
                    static int currentServer = 0;
                    static int currentReferrer = 0;

                    for (const auto& account : regnumAccounts) {
                        ImGui::Text("%s", account.username.c_str());
                        ImGui::NextColumn();

                        // Find and display the server name
                        const char* serverName = account.server.c_str();
                        for (const auto& serverOption : serverOptions) {
                            if (strcmp(serverOption.id, account.server.c_str()) == 0) {
                                serverName = serverOption.name;
                                break;
                            }
                        }
                        ImGui::Text("%s", serverName);
                        ImGui::NextColumn();

                        // Find and display the referrer name
                        const char* referrerName = account.referrer.c_str();
                        for (const auto& referrerOption : referrerOptions) {
                            if (strcmp(referrerOption.id, account.referrer.c_str()) == 0) {
                                referrerName = referrerOption.name;
                                break;
                            }
                        }
                        ImGui::Text("%s", referrerName);
                        ImGui::NextColumn();

                        // button to edit the account, this will load the account details into the input fields
                        std::string editButtonLabel = "Edit##" + std::to_string(account.id);
                        if (ImGui::Button(editButtonLabel.c_str())) {
                            // Load the account details into the input fields
                            snprintf(regnumId, IM_ARRAYSIZE(regnumId), "%d", account.id);
                            snprintf(regnumUsername, IM_ARRAYSIZE(regnumUsername), "%s", account.username.c_str());
                            snprintf(regnumPassword, IM_ARRAYSIZE(regnumPassword), "%s", account.password.c_str());
                            snprintf(regnumServer, IM_ARRAYSIZE(regnumServer), "%s", account.server.c_str());
                            snprintf(regnumReferrer, IM_ARRAYSIZE(regnumReferrer), "%s", account.referrer.c_str());

                            // Set the current server and referrer indices
                            for (int i = 0; i < IM_ARRAYSIZE(serverOptions); ++i) {
                                if (strcmp(serverOptions[i].id, account.server.c_str()) == 0) {
                                    currentServer = i;
                                    break;
                                }
                            }
                            for (int i = 0; i < IM_ARRAYSIZE(referrerOptions); ++i) {
                                if (strcmp(referrerOptions[i].id, account.referrer.c_str()) == 0) {
                                    currentReferrer = i;
                                    break;
                                }
                            }
                        }

                        ImGui::SameLine();
                        // button to delete the account using the DeleteRegnumAccount function
                        std::string deleteButtonLabel = "Delete##" + std::to_string(account.id);
                        if (ImGui::Button(deleteButtonLabel.c_str())) {
                            DeleteRegnumAccount(account.id);
                        }

                        ImGui::NextColumn();
                    }

                    ImGui::Columns(1);
                    ImGui::Separator();

                    // Input fields to save a Regnum Account using the SaveRegnumAccount function
                    ImGui::InputText("Username", regnumUsername, IM_ARRAYSIZE(regnumUsername));
                    ImGui::InputText("Password", regnumPassword, IM_ARRAYSIZE(regnumPassword), ImGuiInputTextFlags_Password);
                    ImGui::Combo("Server", &currentServer, [](void* data, int idx, const char** out_text) {
                        *out_text = ((ServerOption*)data)[idx].name;
                        return true;
                    }, serverOptions, IM_ARRAYSIZE(serverOptions));
                    ImGui::Combo("Referrer", &currentReferrer, [](void* data, int idx, const char** out_text) {
                        *out_text = ((ReferrerOption*)data)[idx].name;
                        return true;
                    }, referrerOptions, IM_ARRAYSIZE(referrerOptions));

                    if (ImGui::Button("Save Account")) {
                        SaveRegnumAccount(
                            regnumUsername, 
                            regnumPassword, 
                            serverOptions[currentServer].id, 
                            referrerOptions[currentReferrer].id, 
                            regnumId[0] == '\0' ? 0 : atoi(regnumId)
                        );
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    static int selectedAccount = -1;
                    const char* exampleAccounts[] = { "Account1", "Account2", "Account3" };
                    if (ImGui::BeginCombo("##Select Account", selectedAccount == -1 ? "Select an account" : exampleAccounts[selectedAccount])) {
                        for (int i = 0; i < IM_ARRAYSIZE(exampleAccounts); i++) {
                            bool isSelected = (selectedAccount == i);
                            if (ImGui::Selectable(exampleAccounts[i], isSelected)) {
                                selectedAccount = i;
                            }
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Play")) {
                        runRoClientGame(regnumLoginUser, regnumLoginPassword);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // slider to set sound volume
                    static float soundVolume = 0.5f;
                    ImGui::SliderFloat("Sound Volume", &soundVolume, 0.0f, 1.0f);

                    // checkbox to enable/disable music
                    static bool enableMusic = true;
                    ImGui::Checkbox("Enable Music", &enableMusic);

                    // checkbox to enable/disable sound effects
                    static bool enableSoundEffects = true;
                    ImGui::Checkbox("Enable Sound Effects", &enableSoundEffects);

                    // button to save the settings
                    if (ImGui::Button("Save Settings")) {
                        // Implement the logic to save the settings
                    }

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
        // Only process the hotkey if the checkbox is checked
        if (optionFov && userDefinedHotkey != 0 && IsHotkeyPressed(userDefinedHotkey)) {
            fovToggled = !fovToggled; // Toggle the FOV state
            float newValue = fovToggled ? 0.02999999933f : 0.01745329238f;
            MemoryManipulation("fov", newValue);
            // Add a small delay to prevent rapid toggling
            Sleep(200);
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

        if (show_loading_screen) {
            ShowLoadingScreen(show_loading_screen, statusMessage, loginSuccess);
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

    if (myTexture) {
        myTexture->Release();
        myTexture = nullptr;
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
