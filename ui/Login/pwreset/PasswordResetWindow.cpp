#include "PasswordResetWindow.h"
#include "../../../libs/imgui/imgui.h"
#include <windows.h>

// Function to handle the Password Reset Window
void ShowPasswordResetWindow(bool& show_password_reset_window, bool& show_login_window, bool& show_forgot_password_window) {
    static bool tokenWindowIsOpen = true;

    // Begin the ImGui window
    ImGui::Begin("Enter Token and New Password", &tokenWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    // If the window is closed, save settings and quit
    if (!tokenWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char passwordResetToken[256] = "";
    static char newPassword[128] = "";

    // Input field for token
    ImGui::InputText("Token", passwordResetToken, IM_ARRAYSIZE(passwordResetToken));
    ImGui::SameLine();
    ShowHelpMarker("The token was sent to your e-mail address. If you did not receive it, please check your spam folder or request a new token.");

    // Input field for new password
    ImGui::InputText("New Password", newPassword, IM_ARRAYSIZE(newPassword), ImGuiInputTextFlags_Password);

    // Function to handle the submit button click
    auto handleSubmit = [&]() {
        if (SetNewPassword(passwordResetToken, newPassword)) {
            show_password_reset_window = false;
            show_login_window = true;
        } else {
            ImGui::Text("Failed to update password. Please try again.");
        }
    };

    // Submit button
    if (ImGui::Button("Submit")) {
        handleSubmit();
    }

    // Button to request a new token
    if (ImGui::Button("Request new token")) {
        show_password_reset_window = false;
        show_forgot_password_window = true;
    }

    // Button to navigate back to login window
    if (ImGui::Button("Back to Login")) {
        show_password_reset_window = false;
        show_login_window = true;
    }

    // End the ImGui window
    ImGui::End();
}