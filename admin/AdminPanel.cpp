// AdminPanel.cpp
#include "../libs/imgui/imgui.h"
#include "UsersTable.h" // Adjust this path if necessary
#include <random>
#include <string>

std::string currentStatus = "Undetected"; // Default status

std::string GenerateRandomKey() {
    const std::string prefix = "Sylent-X-";
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int key_length = 16; // Length of the random part of the key

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string random_key = prefix;
    for (int i = 0; i < key_length; ++i) {
        random_key += characters[distribution(generator)];
    }

    return random_key;
}

void ShowAdminPanel(bool* show_admin_window) {
    static std::string generated_key;
    static int selected_key_type = 0;
    static std::string selected_key_type_str;

    static const char* items[] = { "Fov", "Flyhack", "Moonjump", "Moonwalk", "Fakelag", "Freecam" };
    static bool item_checked[IM_ARRAYSIZE(items)] = { true, true, true, true, true, true };

    static const char* statuses[] = { "Undetected", "Updating", "Detected", "Offline" };
    static int selected_status = 1; // Default to "Online"

    if (*show_admin_window) {
        ImGui::Begin("Admin Panel", show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("All Users");
        DisplayUsersTable();
        ImGui::Separator();

        // Dropdown menu for key type selection
        const char* key_types[] = { "Lifetime Key", "1 Month License Key" };
        if (ImGui::Combo("Key Type", &selected_key_type, key_types, IM_ARRAYSIZE(key_types))) {
            selected_key_type_str = key_types[selected_key_type];
        }

        ImGui::SameLine();

        // Dropdown menu with checkable items
        if (ImGui::BeginCombo("Features", "Select License")) {
            static bool check_all = true;
            if (ImGui::Checkbox("Check/Uncheck All", &check_all)) {
                for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                    item_checked[i] = check_all;
                }
            }
            ImGui::Separator();

            for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                ImGui::Checkbox(items[i], &item_checked[i]);
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        // Dropdown menu for status selection
        if (ImGui::Combo("Current Status", &selected_status, statuses, IM_ARRAYSIZE(statuses))) {
            currentStatus = statuses[selected_status];
        }

        bool any_feature_selected = false;
        for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
            if (item_checked[i]) {
                any_feature_selected = true;
                break;
            }
        }

        if (!any_feature_selected) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Please select at least one feature.");
        }

        if (!selected_key_type_str.empty() && any_feature_selected) {
            if (ImGui::Button("Generate License Key")) {
                generated_key = GenerateRandomKey();
                if (selected_key_type == 1) {
                    generated_key += "-1M";
                }
            }
        } else {
            ImGui::Button("Generate License Key");
        }

        ImGui::SameLine();
        if (!generated_key.empty()) {
            ImGui::InputText("Generated Key", &generated_key[0], generated_key.size() + 1, ImGuiInputTextFlags_ReadOnly);
        }

        ImGui::End();
    }
}