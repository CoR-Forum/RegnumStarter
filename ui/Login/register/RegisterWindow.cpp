#include "RegisterWindow.h"
#include "../../../includes/API/ApiHandler.h"
#include "../../../ui/WindowStates.h"

void ShowRegisterWindow(bool& show_register_window, bool& show_login_window, bool& done) {
    static bool registerWindowIsOpen = true;
    ImGui::Begin("Register", &registerWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    if (!registerWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    static char regUsername[128] = "";
    static char regPassword[128] = "";
    static char regEmail[128] = "";
    static char regNickname[16] = "";

    ImGui::TextEx("Register a new account");

    ImGui::InputTextWithHint("##Username", "Username", regUsername, IM_ARRAYSIZE(regUsername));
    ImGui::SameLine();
    ShowHelpMarker("Your username is only used for logging in and is only visible to you.");

    ImGui::InputTextWithHint("##Nickname", "Nickname", regNickname, IM_ARRAYSIZE(regNickname));
    ImGui::SameLine();
    ShowHelpMarker("Your nickname is visible to other users in chat.");

    ImGui::InputTextWithHint("##Password", "Password", regPassword, IM_ARRAYSIZE(regPassword), ImGuiInputTextFlags_Password);
    ImGui::InputTextWithHint("##Email", "E-Mail Address", regEmail, IM_ARRAYSIZE(regEmail));

    if (ImGui::Button("Register")) {
        RegisterUser(regUsername, regNickname, regEmail, regPassword);
        show_register_window = false;
        show_login_window = true;
    }

    ImGui::Separator();

    if (ImGui::Button("Back to Login")) {
        show_register_window = false;
        show_login_window = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Exit")) {
        done = true;
    }

    ImGui::End();
}