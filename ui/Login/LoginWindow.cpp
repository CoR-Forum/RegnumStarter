#include "LoginWindow.h"
#include "../WindowStates.h" // Include the header file where the window state variables are declared
#include "../../libs/ImageLoader/ImageLoader.h" // Include the ImageLoader header

void ShowLoginWindow(bool& show_login_window, std::string& statusMessage, bool& loginSuccess, bool& show_main_window, ImVec4 textColor) {
    static bool settingsWindowIsOpen = true;
    ImGui::Begin("Login", &settingsWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    
    if (!settingsWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char username[128] = "";
    static char password[128] = "";

    if (saveUsername) {
        strncpy(username, login.c_str(), sizeof(username));
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

    ImGui::InputTextWithHint("##Username", "Username", username, IM_ARRAYSIZE(username));

    ImGui::InputTextWithHint("##Password", "Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

    bool loginTriggered = ImGui::Button("Login", ImVec2(100, 0)); // Adjust the width as needed
    ImGui::SameLine();
    ImGui::Checkbox("Save Username", &saveUsername);

    // Check for Enter key press
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
        loginTriggered = true;
    }

    if (loginTriggered) {
        statusMessage = "Logging in...";

        // Perform the login operation
        loginSuccess = Login(username, password);

        if (loginSuccess) {
            statusMessage = "Login successful";
            show_login_window = false;
            show_main_window = true;
        } else {
            statusMessage = "Login failed";
        }
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