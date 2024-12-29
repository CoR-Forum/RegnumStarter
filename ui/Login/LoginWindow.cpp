#include "LoginWindow.h"
#include "../WindowStates.h"
#include "../../libs/ImageLoader/ImageLoader.h"
#include <windows.h> // Include the Windows API header

void ShowLoginWindow(bool& show_login_window, std::string& statusMessage, bool& loginSuccess, bool& show_main_window, ImVec4 textColor) {
    static bool settingsWindowIsOpen = true;
    static bool focusSet = false;
    static bool usernameSet = false;
    static bool isLoading = false;
    static bool loginTriggered = false;

    std::string windowTitle = "Login - " + regnumstarter_windowname;
    ImGui::Begin(windowTitle.c_str(), &settingsWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    
    if (!settingsWindowIsOpen) {
        SaveSettings();
        SaveLoginSettings(login, saveUsername);
        PostQuitMessage(0);
    }

    static char username[128] = "";
    static char password[128] = "";

    // Set the username from saved settings only once
    if (saveUsername && !usernameSet) {
        strncpy(username, login.c_str(), sizeof(username));
        usernameSet = true;
    }

    // Load the regnumstarter icon from the resources
    static LPDIRECT3DTEXTURE9 texture_regnumstarter_logo = nullptr;
    if (!texture_regnumstarter_logo) {
        texture_regnumstarter_logo = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_REGNUMSTARTER_ICON); 
    }

    // Load the cor-forum logo from the resources
    static LPDIRECT3DTEXTURE9 texture_corforum_logo = nullptr;
    if (!texture_corforum_logo) {
        texture_corforum_logo = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_CORFORUM_LOGO);
    }

    // Display the regnumstarter icon and cor-forum logo side by side
    ImGui::Image((void*)texture_regnumstarter_logo, ImVec2(40, 40));
    ImGui::SameLine();
    ImGui::Image((void*)texture_corforum_logo, ImVec2(174, 40));

    ImGui::Spacing();

    if (setting_log_debug) {
        ImGui::Combo("##API", &apiSelection, apiOptions, IM_ARRAYSIZE(apiOptions));
    }

    // Set focus on the username or password field only once
    if (!focusSet) {
        if (saveUsername && username[0] != '\0') {
            ImGui::SetKeyboardFocusHere(1);
        } else {
            ImGui::SetKeyboardFocusHere(0);
        }
        focusSet = true;
    }

    ImGui::Text("Login with your cor-forum.de account");
    
    ImGui::InputTextWithHint("##Username", "Username", username, IM_ARRAYSIZE(username), showUsername ? 0 : ImGuiInputTextFlags_Password);

    ImGui::InputTextWithHint("##Password", "Password", password, IM_ARRAYSIZE(password), showPassword ? 0 : ImGuiInputTextFlags_Password);

    // Check if Caps Lock is enabled
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Caps Lock is enabled");
    }

    ImGui::BeginDisabled(isLoading);
    if (ImGui::Button("Login", ImVec2(240, 30))) {
        loginTriggered = true;
    }
    ImGui::EndDisabled();

    if (ImGui::BeginPopup("OptionsPopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Checkbox("Save Username", &saveUsername);
        ImGui::Checkbox("Show Username", &showUsername);
        ImGui::Checkbox("Show Password", &showPassword);
        // Add a close button
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Display the status message underneath the login button if it's set
    if (!statusMessage.empty()) {
        ImGui::TextColored(ImVec4(0.75f, 0.0f, 0.75f, 1.0f), "%s", statusMessage.c_str());
    }

    // Check for Enter key press
    if (!isLoading && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
        loginTriggered = true;
    }

    if (loginTriggered && !isLoading) {
        statusMessage = "Logging in...";
        isLoading = true;
        loginTriggered = false;
        ImGui::End();
        // Perform the login operation asynchronously
        std::thread([&]() {
            auto loginResult = Login(username, password);
            loginSuccess = loginResult.first;
            statusMessage = loginResult.second;

            isLoading = false;

            if (loginSuccess) {
                show_login_window = false;
                show_main_window = true;
            } else {
                focusSet = false;
            }
        }).detach();
        return;
    }

    ImGui::Separator();
    
    if (ImGui::Button("Create Account")) {
        ShellExecute(0, 0, "https://api.regnumstarter.cor-forum.de/v1/register", 0, 0, SW_SHOW);
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset Password")) {
        ShellExecute(0, 0, "https://api.regnumstarter.cor-forum.de/v1/reset-password", 0, 0, SW_SHOW);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_COG)) {
        ImGui::OpenPopup("OptionsPopup");
    }

    ImGui::End();
}