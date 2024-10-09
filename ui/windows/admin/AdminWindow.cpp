#include "../../../../../includes/Utils.h"
#include "AdminWindow.h"
#include "UsersTable.h"

void ShowAdminWindow(bool* show_admin_window) {
    if (*show_admin_window) {
                ImGui::Begin("Admin Panel", &show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

                ImGui::Text("All Users");

                DisplayUsersTable();
                            
                ImGui::End();
    }
}