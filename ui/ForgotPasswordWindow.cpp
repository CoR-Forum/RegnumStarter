#include "ForgotPasswordWindow.h"
#include "../libs/imgui/imgui.h"
#include <windows.h>

/**
 * @brief Displays the "Forgot Password" window and handles user interactions.
 *
 * This function creates and manages the "Forgot Password" window using ImGui. It allows users to enter their email address
 * to request a password reset link. The window provides options to request a password reset, indicate that the user already
 * has a reset token, or return to the login window.
 *
 * @param show_forgot_password_window A reference to a boolean that controls the visibility of the "Forgot Password" window.
 * @param show_password_reset_window A reference to a boolean that controls the visibility of the "Password Reset" window.
 * @param show_login_window A reference to a boolean that controls the visibility of the "Login" window.
 */
void ShowForgotPasswordWindow(bool& show_forgot_password_window, bool& show_password_reset_window, bool& show_login_window) {
    static bool forgotpassWindowIsOpen = true;

    ImGui::Begin("Forgot Password", &forgotpassWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    if (!forgotpassWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    ImGui::Text("Enter your email address to receive a password reset link.");
    
    ImGui::Spacing();
    
    ImGui::InputTextWithHint("##Email", "E-Mail Address", forgotPasswordEmail, IM_ARRAYSIZE(forgotPasswordEmail));

    auto handleSubmit = [&]() {
        if (ResetPasswordRequest(forgotPasswordEmail)) {
            show_forgot_password_window = false;
            show_password_reset_window = true;
        } else {
            ImGui::Text("Failed to send reset password request. Please check your email and try again.");
        }
    };

    ImGui::SameLine();
    if (ImGui::Button("Request Password Reset")) {
        handleSubmit();
    }

    ImGui::Separator();

    if (ImGui::Button("I already have a token")) {
        show_forgot_password_window = false;
        show_password_reset_window = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Back to Login")) {
        show_forgot_password_window = false;
        show_login_window = true;
    }

    ImGui::End();
}