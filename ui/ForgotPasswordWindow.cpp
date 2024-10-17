
#include "ForgotPasswordWindow.h"
#include "../libs/imgui/imgui.h"
#include <windows.h>

void ShowForgotPasswordWindow(bool& show_forgot_password_window, bool& show_password_reset_window, bool& show_login_window) {
    static bool forgotpassWindowIsOpen = true;

    ImGui::Begin("Forgot Password", &forgotpassWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    if (!forgotpassWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char forgotPasswordEmail[128] = "";

    ImGui::InputText("Email", forgotPasswordEmail, IM_ARRAYSIZE(forgotPasswordEmail));

    if (ImGui::Button("Submit")) {
        if (ResetPasswordRequest(forgotPasswordEmail)) {
            show_forgot_password_window = false;
            show_password_reset_window = true;
        } else {
            ImGui::Text("Failed to send reset password request. Please try again.");
        }
    }

    if (ImGui::Button("I already have a token")) {
        show_forgot_password_window = false;
        show_password_reset_window = true;
    }

    if (ImGui::Button("Back to Login")) {
        show_forgot_password_window = false;
        show_login_window = true;
    }

    ImGui::End();
}