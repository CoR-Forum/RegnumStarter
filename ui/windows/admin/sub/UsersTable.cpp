#include "../../../../../includes/Utils.h"

void DisplayUsersTable() {
    // Parse the JSON data
    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(GetAllUsersRawJson);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    // Check if jsonData contains the "users" array
    if (!jsonData.contains("users") || !jsonData["users"].is_array()) {
        std::cerr << "Expected JSON array 'users' but got: " << jsonData.type_name() << std::endl;
        return;
    }

    // Begin the ImGui table with a maximum height
    ImGui::BeginChild("UsersTableChild", ImVec2(800, 600), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::BeginTable("AllUsersTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 20.0f);
        ImGui::TableSetupColumn("Username", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Email", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Role", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableHeadersRow();

        // Iterate over the user data and populate the table rows
        for (const auto& user : jsonData["users"]) {
            if (!user.is_object()) {
                std::cerr << "Expected JSON object but got: " << user.type_name() << std::endl;
                continue;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", user.value("id", 0));
            ImGui::TableNextColumn();
            ImGui::Text("%s", user.value("username", "N/A").c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", user.value("email", "N/A").c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", user.value("is_admin", 0) ? "Admin" : "User");
            ImGui::TableNextColumn();
            // if is_active, show disable button, else show enable button
            if (user.value("is_active", 0)) {
                if (ImGui::Button(("Enabled##" + std::to_string(user.value("id", 0))).c_str())) {
                    // Placeholder for disable logic
                    std::cout << "User " << user.value("username", "N/A") << " disabled." << std::endl;
                }
            } else {
                if (ImGui::Button(("Disabled##" + std::to_string(user.value("id", 0))).c_str())) {
                    // Placeholder for enable logic
                    std::cout << "User " << user.value("username", "N/A") << " enabled." << std::endl;
                }
            }
            ImGui::TableNextColumn();
            // button to call ToggleUserBan with user["id"]
            if (user.value("is_banned", 0)) {
                if (ImGui::Button(("Unban##" + std::to_string(user.value("id", 0))).c_str())) {
                    ToggleUserBan(user.value("id", 0));
                }
            } else {
                if (ImGui::Button(("Ban##" + std::to_string(user.value("id", 0))).c_str())) {
                    ToggleUserBan(user.value("id", 0));
                }
            }
        }
    
    ImGui::EndTable();
    ImGui::EndChild();
}