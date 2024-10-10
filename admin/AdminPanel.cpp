// AdminPanel.cpp
#include "../libs/imgui/imgui.h"
#include "UsersTable.h" // Adjust this path if necessary
#include <random>
#include <string>

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
    static std::string generated_key; // Static variable to store the generated key
    static int selected_key_type = 0; // 0 for Lifetime, 1 for 1 Month
    static std::string selected_key_type_str; // Static variable to store the selected key type as string

    // Define items for the dropdown menu and their checked state
    static const char* items[] = { "Fov", "Flyhack", "Moonjump", "Moonwalk", "Fakelag", "Freecam" };
    static bool item_checked[IM_ARRAYSIZE(items)] = { true, true, true, true, true, true }; // All items checked by default

    if (*show_admin_window) {
        ImGui::Begin("Admin Panel", show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("All Users");
        DisplayUsersTable();
        ImGui::Separator();

        // Dropdown menu for key type selection
        const char* key_types[] = { "Lifetime Key", "1 Month License Key" };
        if (ImGui::Combo("Key Type", &selected_key_type, key_types, IM_ARRAYSIZE(key_types))) {
            selected_key_type_str = key_types[selected_key_type]; // Update the selected key type string
        }

        ImGui::SameLine(); // Place the next item on the same line

        // Dropdown menu with checkable items
        if (ImGui::BeginCombo("Features", "Select License")) {
            for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                ImGui::Checkbox(items[i], &item_checked[i]);
            }
            ImGui::EndCombo();
        }

        // Check if at least one feature is selected
        bool any_feature_selected = false;
        for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
            if (item_checked[i]) {
                any_feature_selected = true;
                break;
            }
        }

        // Disable the button if no key type is selected or no features are selected
        if (!selected_key_type_str.empty() && any_feature_selected) {
            if (ImGui::Button("Generate License Key")) {
                generated_key = GenerateRandomKey();
                if (selected_key_type == 1) {
                    generated_key += "-1M"; // Append "-1M" for 1 Month License Key
                }
            }
        } else {
            ImGui::Button("Generate License Key"); // Render the button in a disabled state
        }

        ImGui::SameLine(); // Display the text on the same line as the button
        if (!generated_key.empty()) {
            ImGui::InputText("Generated Key", &generated_key[0], generated_key.size() + 1, ImGuiInputTextFlags_ReadOnly);
        }
        ImGui::End();
    }
}