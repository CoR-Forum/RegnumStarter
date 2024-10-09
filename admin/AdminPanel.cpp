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

    if (*show_admin_window) {
        ImGui::Begin("Admin Panel", show_admin_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("All Users");
        DisplayUsersTable();
        ImGui::Separator();
        if (ImGui::Button("Generate Random Key")) {
            generated_key = GenerateRandomKey();
        }
        ImGui::SameLine(); // Display the text on the same line as the button
        if (!generated_key.empty()) {
            ImGui::Text("Generated Key: %s", generated_key.c_str());
        }
        ImGui::End();
    }
}