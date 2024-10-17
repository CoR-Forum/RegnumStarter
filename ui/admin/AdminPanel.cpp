#include "AdminPanel.h"
#include "AdminPanelTables.h" // Include the new header

// Global variables
std::string currentStatus = "Undetected"; // Default status
std::string generatedLicenseKey = ""; // Initialize the global variable

// Function to display the Admin Panel window
void ShowAdminPanel(bool* show_admin_window) {
    extern std::string generatedLicenseKey;
    static int selected_key_runtime = 0; // Default to the first runtime
    static std::string selected_key_runtime_str = "1h"; // Default runtime string

    static const char* key_features[] = { "zoom", "fov", "gravity", "moonjump", "moonwalk", "fakelag", "fakelagg", "freecam", "speedhack" };
    static bool default_key_features_selected[IM_ARRAYSIZE(key_features)] = { true, true, true, true, true, true, true, true };

    static const char* statuses[] = { "Undetected", "Updating", "Detected", "Offline" };
    static int selected_status = 1; // Default to "Online"

    if (*show_admin_window) {
        ImGui::Begin("Administration", show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        // Display Users Table
        ImGui::Text("All Users");
        DisplayUsersTable();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Display Licenses Table
        ImGui::Text("All Licenses");
        DisplayLicensesTable();

        ImGui::Spacing();

        // Generate License Key Section
        ImGui::Text("Generate License Key");

        bool any_feature_selected = false;
        for (int i = 0; i < IM_ARRAYSIZE(key_features); i++) {
            if (default_key_features_selected[i]) {
                any_feature_selected = true;
                break;
            }
        }

        // Dropdown menu for key runtime selection
        const char* key_runtimes[] = { "1h", "1d", "7d", "1m", "3m", "10y" };
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

        // Button to generate license key
        if (ImGui::Button("Generate License Key")) {
            std::string licensedFeatures;
            for (int i = 0; i < IM_ARRAYSIZE(key_features); i++) {
                if (default_key_features_selected[i]) {
                    licensedFeatures += key_features[i];
                    licensedFeatures += ",";
                }
            }

            if (!licensedFeatures.empty()) {
                licensedFeatures.pop_back(); // Remove the trailing comma
                GenerateNewLicense(licensedFeatures, selected_key_runtime_str);
            }
        }

        // Display the generated license key
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

        // Checkbox for debug logging
        ImGui::Checkbox("Debug Log", &setting_log_debug);

        ImGui::End();
    }
}