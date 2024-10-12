// AdminPanel.cpp
#include "../libs/imgui/imgui.h"
#include "UsersTable.h" // Adjust this path if necessary
#include <random>
#include <string>
#include "AdminPanel.h"

std::string currentStatus = "Undetected"; // Default status
std::string generatedLicenseKey = ""; // Initialize the global variable

void ShowAdminPanel(bool* show_admin_window) {
    extern std::string generatedLicenseKey;
    static int selected_key_runtime;
    static std::string selected_key_runtime_str;

    static const char* key_features[] = { "fov", "flyhack", "moonjump", "moonwalk", "fakelag", "freecam", "speedhack", "fastfly"};
    static bool default_key_features_selected[IM_ARRAYSIZE(key_features)] = { true, true, true, true, true, true, true, true };

    static const char* statuses[] = { "Undetected", "Updating", "Detected", "Offline" };
    static int selected_status = 1; // Default to "Online"

    if (*show_admin_window) {
        ImGui::Begin("Admin Panel", show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("All Users");
        DisplayUsersTable();
        ImGui::Separator();

        bool any_feature_selected = false;
        for (int i = 0; i < IM_ARRAYSIZE(key_features); i++) {
            if (default_key_features_selected[i]) {
                any_feature_selected = true;
                break;
            }
        }

        // Dropdown menu for key type selection
        const char* key_runtimes[] = { "99y", "1m" };
        if (ImGui::Combo("Runtime", &selected_key_runtime, key_runtimes, IM_ARRAYSIZE(key_runtimes))) {
            selected_key_runtime_str = key_runtimes[selected_key_runtime];
        }

        ImGui::SameLine();

        // Dropdown to select features
        if (ImGui::BeginCombo("Features", "Select Features")) {
            static bool check_all = true;
            if (ImGui::Checkbox("Check/Uncheck All", &check_all)) {
                for (int i = 0; i < IM_ARRAYSIZE(key_features); i++) {
                    default_key_features_selected[i] = check_all;
                }
            }
            ImGui::Separator();

            for (int i = 0; i < IM_ARRAYSIZE(key_features); i++) {
                ImGui::Checkbox(key_features[i], &default_key_features_selected[i]);
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        if (ImGui::Button("Generate License Key")) {
            // call GenerateNewLicense with key_features and runtime
            std::string licensedFeatures;
            for (int i = 0; i < IM_ARRAYSIZE(key_features); i++) {
                if (default_key_features_selected[i]) {
                    licensedFeatures += key_features[i];
                    licensedFeatures += ",";
                }
            }

            licensedFeatures.pop_back(); // Remove the trailing comma

            GenerateNewLicense(licensedFeatures, selected_key_runtime_str);
        }

        // Add a text field to display the generated license key
        if (!generatedLicenseKey.empty()) {
            ImGui::InputText("Generated License Key", &generatedLicenseKey[0], generatedLicenseKey.size() + 1, ImGuiInputTextFlags_ReadOnly);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Dropdown menu for status selection
        if (ImGui::Combo("Current Status", &selected_status, statuses, IM_ARRAYSIZE(statuses))) {
            currentStatus = statuses[selected_status];
        }

        ImGui::End();
    }
}