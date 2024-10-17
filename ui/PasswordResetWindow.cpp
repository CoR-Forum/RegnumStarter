#include "PasswordResetWindow.h"
#include "../libs/imgui/imgui.h"
#include <windows.h>
#include "helper/Markers/HelpMarker.h"

void ShowPasswordResetWindow(bool& show_password_reset_window, bool& show_login_window, bool& show_forgot_password_window) {
    static bool tokenWindowIsOpen = true;

    ImGui::Begin("Enter Token and New Password", &tokenWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    if (!tokenWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char passwordResetToken[128] = "";
    static char newPassword[128] = "";

    ImGui::InputText("Token", passwordResetToken, IM_ARRAYSIZE(passwordResetToken));
    ImGui::SameLine();
    ShowHelpMarker("Sent to you by e-mail");

    ImGui::InputText("New Password", newPassword, IM_ARRAYSIZE(newPassword), ImGuiInputTextFlags_Password);

    if (ImGui::Button("Submit")) {
        if (SetNewPassword(passwordResetToken, newPassword)) {
            MessageBox(NULL, "Password updated successfully. You may now login.", "Success", MB_ICONINFORMATION);
            show_password_reset_window = false;
            show_login_window = true;
        } else {
            ImGui::Text("Failed to update password. Please try again.");
        }
    }

    if (ImGui::Button("Request new token")) {
        show_password_reset_window = false;
        show_forgot_password_window = true;
    }

    if (ImGui::Button("Back to Login")) {
        show_password_reset_window = false;
        show_login_window = true;
    }

    ImGui::End();
}