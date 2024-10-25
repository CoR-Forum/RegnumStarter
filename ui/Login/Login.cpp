#include "Login.h"
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

    // Load the texture
    static LPDIRECT3DTEXTURE9 logoTexture = nullptr;
    if (!logoTexture) {
        logoTexture = LoadTextureFromResource(g_pd3dDevice, IDR_PNG_SYLENT_LOGO); // Assuming you have a resource ID for the logo
    }

    // Display the texture at the top
    if (logoTexture) {
        ImGui::Image((void*)logoTexture, ImVec2(300, 50)); // Adjust the size as needed
    }

    ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
    ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

    if (ImGui::Button("Login")) {
        statusMessage = "Logging in...";
        loginSuccess = false;
        show_login_window = false; // Hide the login window

        std::thread loginThread([&]() {
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
        });

        loginThread.detach();
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