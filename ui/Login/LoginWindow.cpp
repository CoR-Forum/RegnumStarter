#include "LoginWindow.h"
#include "../WindowStates.h" // Include the header file where the window state variables are declared
#include "../../libs/ImageLoader/ImageLoader.h" // Include the ImageLoader header

#include "LoginWindow.h"
#include "../WindowStates.h" // Include the header file where the window state variables are declared
#include "../../libs/ImageLoader/ImageLoader.h" // Include the ImageLoader header

void ShowLoginWindow(bool& show_login_window, std::string& statusMessage, bool& loginSuccess, bool& show_main_window, ImVec4 textColor) {
    static bool settingsWindowIsOpen = true;
    static bool focusSet = false; // Track if the focus has been set
    static bool usernameSet = false; // Track if the username has been set
    static bool isLoading = false; // Track if the login is in progress
    static bool loginTriggered = false; // Track if the login button was pressed
    static bool showPassword = false; // Track if the password should be shown
    static bool showUsername = false; // Track if the username should be shown

    ImGui::Begin("Login", &settingsWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    
    if (!settingsWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char username[128] = "";
    static char password[128] = "";

    // Set the username from saved settings only once
    if (saveUsername && !usernameSet) {
        strncpy(username, login.c_str(), sizeof(username));
        usernameSet = true;
        showUsername = false; // Obfuscate username if set from config file
    }

    // Load the texture
    static LPDIRECT3DTEXTURE9 texture_sylent_logo = nullptr;
    if (!texture_sylent_logo) {
        texture_sylent_logo = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_SYLENT_LOGO); // Assuming you have a resource ID for the logo
    }

    // Display the texture at the top
    if (texture_sylent_logo) {
        ImGui::Image((void*)texture_sylent_logo, ImVec2(231, 38.5)); // Adjust the size as needed
    }

    ImGui::Spacing();

    ImGui::Combo("##API", &apiSelection, apiOptions, IM_ARRAYSIZE(apiOptions));

    // Set focus on the username or password field only once
    if (!focusSet) {
        if (saveUsername && username[0] != '\0') {
            ImGui::SetKeyboardFocusHere(1); // Focus on the password field
        } else {
            ImGui::SetKeyboardFocusHere(0); // Focus on the username field
        }
        focusSet = true;
    }
    
    ImGui::InputTextWithHint("##Username", "Username", username, IM_ARRAYSIZE(username), showUsername ? 0 : ImGuiInputTextFlags_Password);

    ImGui::SameLine();
    if (ImGui::Button(showUsername ? "Hide" : "Show")) {
        showUsername = !showUsername;
    }

    ImGui::InputTextWithHint("##Password", "Password", password, IM_ARRAYSIZE(password), showPassword ? 0 : ImGuiInputTextFlags_Password);

    ImGui::SameLine();
    if (ImGui::Button(showPassword ? "Hide" : "Show")) {
        showPassword = !showPassword;
    }

    ImGui::BeginDisabled(isLoading); // Disable the login button if loading
    if (ImGui::Button("Login", ImVec2(100, 0))) { // Adjust the width as needed
        loginTriggered = true;
    }
    ImGui::EndDisabled();
    
    ImGui::SameLine();
    ImGui::Checkbox("Save Username", &saveUsername);

    // Display the status message underneath the login button
    ImGui::TextColored(ImVec4(0.75f, 0.0f, 0.75f, 1.0f), "%s", statusMessage.c_str());

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
                focusSet = false; // Reset focus to password field on next frame
            }
        }).detach();
        return; // Return immediately to allow the UI to update
    }

    ImGui::Separator();
    
    if (ImGui::Button("Register")) {
        show_login_window = false;
        show_register_window = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Forgot Password")) {
        show_login_window = false;
        show_forgot_password_window = true;
    }

    ImGui::End();
}