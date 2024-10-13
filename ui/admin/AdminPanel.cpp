#include "AdminPanel.h"

std::string currentStatus = "Undetected"; // Default status
std::string generatedLicenseKey = ""; // Initialize the global variable

void ShowAdminPanel(bool* show_admin_window) {
    extern std::string generatedLicenseKey;
    static int selected_key_runtime = 0; // Default to the first runtime
    static std::string selected_key_runtime_str = "1h"; // Default runtime string

    static const char* key_features[] = { "zoom", "fov", "gravity", "moonjump", "moonwalk", "fakelag", "freecam", "speedhack"};
    static bool default_key_features_selected[IM_ARRAYSIZE(key_features)] = { true, true, true, true, true, true, true, true };

    static const char* statuses[] = { "Undetected", "Updating", "Detected", "Offline" };
    static int selected_status = 1; // Default to "Online"

    if (*show_admin_window) {
        ImGui::Begin("Administration", show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("All Users");
        DisplayUsersTable();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        

        ImGui::Text("All Licenses");
        DisplayLicensesTable();

        ImGui::Spacing();
        
        ImGui::Text("Generate License Key");

        bool any_feature_selected = false;
        for (int i = 0; i < IM_ARRAYSIZE(key_features); i++) {
            if (default_key_features_selected[i]) {
                any_feature_selected = true;
                break;
            }
        }

        // Dropdown menu for key type selection
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