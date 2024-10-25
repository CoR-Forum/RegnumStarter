#include "Login.h"
#include "../WindowStates.h" // Include the header file where the window state variables are declared

void ShowLoginWindow(bool& show_login_window, bool& show_loading_screen, std::string& statusMessage, bool& loginSuccess, bool& show_main_window, ImVec4 textColor) {
    static bool settingsWindowIsOpen = true;
    ImGui::Begin("Login", &settingsWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    
    if (!settingsWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char username[128] = "";
    static char password[128] = "";

    ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
    ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
    
    if (ImGui::Button("Login")) {
        show_loading_screen = true;
        statusMessage = "Logging in...";
        loginSuccess = false;
        show_login_window = false; // Hide the login window

        std::thread loginThread([&]() {
            for (int i = 0; i <= 100; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate progress over 1s
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