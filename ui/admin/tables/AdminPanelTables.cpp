// AdminPanelTables.cpp
#include "../../../libs/imgui/imgui.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <ctime>
#include "../../../libs/json.hpp"
#include "../AdminPanel.h"

// Function to join vector elements with a comma
std::string join(const std::vector<std::string>& elements, const std::string& delimiter = ", ") {
    std::ostringstream os;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it != elements.begin()) {
            os << delimiter;
        }
        os << *it;
    }
    return os.str();
}

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
    ImGui::BeginChild("UsersTableChild", ImVec2(800, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::BeginTable("AllUsersTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 25.0f);
    ImGui::TableSetupColumn("Username", ImGuiTableColumnFlags_WidthFixed, 120.0f);
    ImGui::TableSetupColumn("Email", ImGuiTableColumnFlags_WidthFixed, 200.0f);
    ImGui::TableSetupColumn("Role", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    ImGui::TableSetupColumn("Last Login", ImGuiTableColumnFlags_WidthFixed, 100.0f);
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
        // button to call ToggleUserAdmin with user["id"]
        if (user.value("is_admin", 0)) {
            if (ImGui::Button(("Revoke Admin##" + std::to_string(user.value("id", 0))).c_str())) {
                ToggleUserAdmin(user.value("id", 0));
            }
        } else {
            if (ImGui::Button(("Make Admin##" + std::to_string(user.value("id", 0))).c_str())) {
                ToggleUserAdmin(user.value("id", 0));
            }
        }

        ImGui::TableNextColumn();
        // button to call ToggleUserActivation with user["id"]
        if (user.value("is_active", 0)) {
            if (ImGui::Button(("Deactivate##" + std::to_string(user.value("id", 0))).c_str())) {
                ToggleUserActivation(user.value("id", 0));
            }
        } else {
            if (ImGui::Button(("Activate##" + std::to_string(user.value("id", 0))).c_str())) {
                ToggleUserActivation(user.value("id", 0));
            }
        }
        ImGui::TableNextColumn();
        std::string lastLogin = user.contains("last_login") && !user["last_login"].is_null() ? user["last_login"].get<std::string>() : "N/A";
        ImGui::Text("%s", lastLogin.c_str());
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

void DisplayLicensesTable() {
    std::string rawJson = GetAllLicensesRawJson;
    size_t pos = rawJson.find("}{");
    if (pos != std::string::npos) {
        rawJson = rawJson.substr(pos + 1);
    }

    // Parse the JSON data
    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(rawJson);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    // Check if jsonData contains the "licenses" array
    if (!jsonData.contains("licenses") || !jsonData["licenses"].is_array()) {
        std::cerr << "Expected JSON array 'licenses' but got: " << jsonData.type_name() << std::endl;
        return;
    }

    // Begin the ImGui table with a maximum height
    ImGui::BeginChild("LicensesTableChild", ImVec2(800, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::BeginTable("AllLicensesTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 25.0f);
    ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 140.0f);
    ImGui::TableSetupColumn("Features", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    ImGui::TableSetupColumn("Runtime", ImGuiTableColumnFlags_WidthFixed, 20.0f);
    ImGui::TableSetupColumn("Runtime End", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    ImGui::TableSetupColumn("Activated by", ImGuiTableColumnFlags_WidthFixed, 150.0f);
    ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);

    ImGui::TableHeadersRow();

    for (const auto& license : jsonData["licenses"]) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d", license.value("id", 0));
        ImGui::TableNextColumn();
        ImGui::InputText(("##license_key" + std::to_string(license.value("id", 0))).c_str(), const_cast<char*>(license.value("license_key", "N/A").c_str()), license.value("license_key", "N/A").size() + 1, ImGuiInputTextFlags_ReadOnly);
        ImGui::TableNextColumn();
        
        // Parse and display licensed_features as a comma-separated string
        std::string licensedFeaturesStr = license.value("licensed_features", "[]");
        std::vector<std::string> licensedFeatures = nlohmann::json::parse(licensedFeaturesStr).get<std::vector<std::string>>();
        std::string displayStr = join(licensedFeatures);
        ImGui::Text("%s", displayStr.c_str());
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", displayStr.c_str());
        }
        
        ImGui::TableNextColumn();
        ImGui::Text("%s", license.value("runtime", "N/A").c_str());
        ImGui::TableNextColumn();
        std::string runtimeEnd = license.contains("runtime_end") && license["runtime_end"].is_string() ? license["runtime_end"].get<std::string>() : "N/A";
        ImGui::Text("%s", runtimeEnd.c_str());
        ImGui::TableNextColumn();
        std::string activatedBy = "N/A";
        if (license.contains("activated_by_username") && !license["activated_by_username"].is_null() && license["activated_by_username"].is_string()) {
            activatedBy = license["activated_by_username"].get<std::string>();
            if (license.contains("activated_by") && !license["activated_by"].is_null() && license["activated_by"].is_number()) {
                activatedBy += " (" + std::to_string(license["activated_by"].get<int>()) + ")";
            }
        }
        ImGui::Text("%s", activatedBy.c_str());
        ImGui::TableNextColumn();

        // Check if the license has not expired
        if (runtimeEnd != "N/A") {
            std::tm tm = {};
            std::istringstream ss(runtimeEnd);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
            std::time_t runtimeEndTime = std::mktime(&tm);
            std::time_t currentTime = std::time(nullptr);

            if (runtimeEndTime > currentTime) {
                if (ImGui::Button(("Expire##" + std::to_string(license.value("id", 0))).c_str())) {
                    ExpireLicense(license.value("id", 0));
                }
            }
        }
    }

    ImGui::EndTable();
    ImGui::EndChild();
}