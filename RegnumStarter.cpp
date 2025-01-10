#include "RegnumStarter.h"

bool g_DeviceLost = false;
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
bool spaceKeyPressed = false;
bool ctrlKeyPressed = false;
bool fovToggled = false;

ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

bool g_ShowUI = true;
std::string statusMessage = "";
bool loginSuccess = false;

int userDefinedHotkey = 0;
bool waitingForHotkey = false;

std::vector<Pointer> pointers;

const std::string regnumLoginUser = "username";
const std::string regnumLoginPassword = "password";

bool IsHotkeyPressed(int hotkey) {
    return GetAsyncKeyState(hotkey) & 0x8000;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    
    Log("RegnumStarter " + regnumstarter_version + " started");
    HANDLE hMutex = CreateMutex(NULL, TRUE, _T("RegnumStarter-Mutex")); // Create a named mutex
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hwndExisting = FindWindow(NULL, _T("RegnumStarter"));
        if (hwndExisting) {
            PostMessage(hwndExisting, WM_CLOSE, 0, 0); // Send a close message to the existing instance
            Sleep(2000); // Wait for the existing instance to close
        }
    }
    
    SelfUpdate();
    LoadLoginSettings();

    std::string folderPath = std::string(appDataPath) + "\\RegnumStarter";
    std::filesystem::create_directories(folderPath);

    show_login_window = true;

    // Register and create the main window
    WNDCLASSEXW wc = { sizeof(wc), CS_DBLCLKS | CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"RegnumStarter", nullptr };
    ::RegisterClassExW(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, _T("RegnumStarter"), NULL, WS_POPUP | WS_VISIBLE, 0, 0, 900, 500, NULL, NULL, wc.hInstance, NULL);
    SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    LPDIRECT3DTEXTURE9 texture_sylent_icon = nullptr;
    texture_sylent_icon = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_REGNUMSTARTER_ICON);

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.MouseDrawCursor = false;
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

            if (show_main_window) {
                std::string windowTitle = regnumstarter_windowname;
                static bool mainWindowIsOpen = true; // Add a boolean to control the window's open state
                ImGui::SetNextWindowSize(ImVec2(770, 400), ImGuiCond_FirstUseEver);
                ImGui::Begin(windowTitle.c_str(), &mainWindowIsOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

                static bool captureExclusionSet = false;
                if (!captureExclusionSet) {
                    SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
                    captureExclusionSet = true;
                }

                ImGui::GetStyle().Colors[ImGuiCol_Text] = textColor;
                ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = textColor;

                // close the window if the user clicks the close button
                if (!mainWindowIsOpen) {
                    PostQuitMessage(0);
                }
                
                // Create a child window for the texture without border
                ImGui::BeginChild("TextureChild", ImVec2(130, 90), false);
                if (texture_sylent_icon) {
                    // Calculate the available space
                    ImVec2 availableSpace = ImGui::GetContentRegionAvail();
                    ImVec2 imageSize = ImVec2(60, 60); // Adjust the size as needed

                    // Calculate the padding to center the image
                    ImVec2 padding = ImVec2((availableSpace.x - imageSize.x) * 0.5f, (availableSpace.y - imageSize.y) * 0.5f);

                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding.x);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);

                    ImGui::Image((void*)texture_sylent_icon, imageSize);
                } else {
                    ImGui::Text("Texture is null");
                }
                ImGui::EndChild();

                ImGui::SameLine();
                // Create a child window for the texture
                ImGui::BeginChild("Menu", ImVec2(615, 90), false);
                ImGui::Columns(3, nullptr, false); // Create three columns

                // Left column for account selection with scrollbar
                ImGui::SetColumnWidth(0, 240); // Adjust the width of the first column
                ImGui::BeginChild("AccountSelection", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                if (regnumAccounts.empty()) {
                    ImGui::Text("No accounts available.");
                    ImGui::Text("Please add an account.");
                } else {
                    for (int i = 0; i < regnumAccounts.size(); i++) {
                        bool isSelected = (selectedAccount == i);
                        if (ImGui::Selectable(regnumAccounts[i].username.c_str(), isSelected)) {
                            selectedAccount = i;
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::NextColumn();

                // Middle column for the "Play" button
                ImGui::SetColumnWidth(1, 180); // Adjust the width of the second column
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5); // Add some padding
                if (ImGui::Button("Play", ImVec2(160, 60))) { // Make the button bigger
                    if (selectedAccount != -1) {
                        const auto& account = regnumAccounts[selectedAccount];
                        runRoClientGame(account.username, account.password);
                    }
                }
                ImGui::NextColumn();

                // Right column for "Game Settings" and "Regnum Accounts" buttons
                ImGui::SetColumnWidth(2, 160); // Adjust the width of the third column
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5); // Add some padding
                if (ImGui::Button("Game Settings", ImVec2(160, 25))) {
                    show_settings_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumSettings = true;
                    show_view_window = false;
                    show_calendar_window = false;
                    show_RegnumAccounts = false;
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5); // Add some padding
                if (ImGui::Button("Regnum Accounts", ImVec2(160, 25))) {
                    show_settings_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumSettings = false;
                    show_view_window = false;
                    show_calendar_window = false;
                    show_RegnumAccounts = true;
                }

                ImGui::EndChild();

                ImVec2 buttonSize = ImVec2(120, 30);

                // Create a child window for the navigation buttons
                ImGui::BeginChild("Navigation", ImVec2(140, 0), false);

                // Calculate the padding to center the buttons
                float childWidth = ImGui::GetWindowWidth();
                float buttonPadding = (childWidth - buttonSize.x) / 2.0f;

                ImGui::SetCursorPosX(buttonPadding);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                if (ImGui::Button("Chat", buttonSize)) {
                    show_settings_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumSettings = false;
                    show_view_window = false;
                    show_calendar_window = false;
                    show_RegnumAccounts = false; // Ensure Regnum Accounts window is closed
                }
                ImGui::PopStyleVar();

                ImGui::SetCursorPosX(buttonPadding);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                if (ImGui::Button("Tools4Regnum", buttonSize)) {
                    show_settings_window = false;
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumSettings = false;
                    show_calendar_window = false;
                    show_view_window = true;
                }
                ImGui::PopStyleVar();

                ImGui::SetCursorPosX(buttonPadding);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                if (ImGui::Button("Calendar", buttonSize)) {
                    show_settings_window = false;
                    show_license_window = false;
                    show_RegnumSettings = false;
                    show_view_window = false;
                    show_info_window = false;
                    show_calendar_window = true;
                    InitializeBossRespawns();
                }
                ImGui::PopStyleVar();

                ImGui::SetCursorPosX(buttonPadding);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                if (ImGui::Button("Settings", buttonSize)) {
                    show_license_window = false;
                    show_info_window = false;
                    show_RegnumSettings = false;
                    show_view_window = false;
                    show_calendar_window = false;
                    show_settings_window = true;
                }
                ImGui::PopStyleVar();

                ImGui::SetCursorPosX(buttonPadding);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
                if (ImGui::Button("Logout", buttonSize)) {
                    Logout();
                }
                ImGui::PopStyleVar();

                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::BeginChild("MainContent", ImVec2(0, 0), true);

                if (show_settings_window) {

                    ImGui::SeparatorText("General");

                    if (ImGui::Checkbox("Streamproof", &setting_excludeFromCapture)) {
                        SetWindowCaptureExclusion(hwnd, setting_excludeFromCapture);
                    }
                    ImGui::SameLine();
                    ShowHelpMarker("Exclude the window from screen capture and hide from taskbar");

                    ImGui::SeparatorText("Appearance");

                    static ImVec4 backupColor = textColor;

                    if (ImGui::Button("Font Color")) {
                        backupColor = textColor; // Backup the current color
                        ImGui::OpenPopup("Font Color Picker");
                    }

                    if (ImGui::BeginPopup("Font Color Picker")) {
                        ImGui::Text("Font Color");
                        ImGui::ColorPicker4("##picker", (float*)&textColor);
                        if (ImGui::Button("Save")) {
                            SaveSettings();
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel")) {
                            textColor = backupColor; // Revert to the backup color
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Save Settings")) {
                        SaveSettings();
                    }

            } else if (show_license_window) {
                ShowLicenseWindow(show_license_window);

            } else if (show_info_window) {
                ShowCreditsWindow(show_info_window);  

            } else if (show_view_window) {
                ShowViewWindow(show_view_window, optionZoom, optionFov, featureFov, featureZoom, waitingForHotkey, userDefinedHotkey);
            } else if (show_RegnumSettings) {
                ShowRegnumSettings(show_RegnumSettings);
            } else if (show_calendar_window) {
                ShowBossRespawnWindow(show_calendar_window);
            } else if (show_RegnumAccounts) {
                ShowRegnumAccounts(show_RegnumAccounts);
            } else {
                    ImGui::GetStyle().Colors[ImGuiCol_Text] = textColor;
                    ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = textColor;
                    // Log and chat display box at the bottom
                    ImGui::BeginChild("LogMessages", ImVec2(580, 200), false, ImGuiWindowFlags_HorizontalScrollbar);
                    for (const auto& msg : logMessages) {
                        ImGui::TextWrapped("%s", msg.c_str());
                    }
                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                        ImGui::SetScrollHereY(1.0f); // Scroll to the bottom
                    }
                    ImGui::EndChild();

                    // input field and button to send chat messages using sendChatMessage function
                    ImGui::PushItemWidth(500); // Set the width of the input field
                    if (ImGui::InputTextWithHint("##ChatInput", "Type your message...", chatInput, IM_ARRAYSIZE(chatInput), ImGuiInputTextFlags_EnterReturnsTrue)) {
                        if (strlen(chatInput) > 0) {
                            SendChatMessage(chatInput);
                            chatInput[0] = '\0'; // Clear input field
                                                        ImGui::SetKeyboardFocusHere(-1); // Keep focus on the input field

                        }
                    }
                    ImGui::PopItemWidth(); // Reset the item width to default
                    ImGui::SameLine();
                    if (ImGui::Button("Send") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
                        if (strlen(chatInput) > 0) {
                            SendChatMessage(chatInput);
                            chatInput[0] = '\0'; // Clear input field
                                                        ImGui::SetKeyboardFocusHere(-1); // Keep focus on the input field

                        }
                    }
                }
                ImGui::EndChild();
                ImGui::End();
            }

            if (show_RegnumAccounts) {
                ShowRegnumAccounts(show_RegnumAccounts);
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

    Log("RegnumStarter exiting");
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
        g_ResizeWidth = (UINT)LOWORD(lParam);
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
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
        if (hit == HTCLIENT && !ImGui::IsAnyItemHovered()) {
            hit = HTCAPTION;
        }
        return hit;
    }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

HANDLE hProcess = nullptr; // Handle to the target process (ROClientGame.exe)
DWORD pid; // Process ID of the target process


std::atomic<bool> isWriting(false);
std::thread memoryThread;