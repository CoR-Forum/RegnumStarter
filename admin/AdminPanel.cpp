// AdminPanel.cpp
#include "../libs/imgui/imgui.h"
#include "UsersTable.h" // Adjust this path if necessary

void ShowAdminPanel(bool* show_admin_window) {
    if (*show_admin_window) {
        ImGui::Begin("Admin Panel", show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("All Users");
        DisplayUsersTable();
        ImGui::End();
    }
}