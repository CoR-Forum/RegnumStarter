#include "PasswordResetWindow.h"
#include "../libs/imgui/imgui.h"
#include <windows.h>
#include "helper/Markers/HelpMarker.h"

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

    static char passwordResetToken[128] = "";
    static char newPassword[128] = "";

    // Input field for token
    ImGui::InputText("Token", passwordResetToken, IM_ARRAYSIZE(passwordResetToken));
    ImGui::SameLine();
    ShowHelpMarker("Sent to you by e-mail");

    // Input field for new password
    ImGui::InputText("New Password", newPassword, IM_ARRAYSIZE(newPassword), ImGuiInputTextFlags_Password);

    // Function to handle the submit button click
    auto handleSubmit = [&]() {
        if (SetNewPassword(passwordResetToken, newPassword)) {
            MessageBox(NULL, "Password updated successfully. You may now login.", "Success", MB_ICONINFORMATION);
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