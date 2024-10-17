#include "ForgotPasswordWindow.h"
#include "../libs/imgui/imgui.h"
#include <windows.h>

// Function to handle the Forgot Password Window
void ShowForgotPasswordWindow(bool& show_forgot_password_window, bool& show_password_reset_window, bool& show_login_window) {
    static bool forgotpassWindowIsOpen = true;

    // Begin the ImGui window
    ImGui::Begin("Forgot Password", &forgotpassWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    // If the window is closed, save settings and quit
    if (!forgotpassWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char forgotPasswordEmail[128] = "";

    // Input field for email
    ImGui::InputText("Email", forgotPasswordEmail, IM_ARRAYSIZE(forgotPasswordEmail));

    // Function to handle the submit button click
    auto handleSubmit = [&]() {
        if (ResetPasswordRequest(forgotPasswordEmail)) {
            show_forgot_password_window = false;
            show_password_reset_window = true;
        } else {
            ImGui::Text("Failed to send reset password request. Please check your email and try again.");
        }
    };

    // Submit button
    if (ImGui::Button("Submit")) {
        handleSubmit();
    }

    // Button to navigate to password reset window
    if (ImGui::Button("I already have a token")) {
        show_forgot_password_window = false;
        show_password_reset_window = true;
    }

    // Button to navigate back to login window
    if (ImGui::Button("Back to Login")) {
        show_forgot_password_window = false;
        show_login_window = true;
    }

    // End the ImGui window
    ImGui::End();
}