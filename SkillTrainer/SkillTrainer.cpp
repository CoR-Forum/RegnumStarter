#include "SkillTrainer.h"
#include <map>
#include "../ui/WindowStates.h"
#include "../libs/ImageLoader/ImageLoader.h"
#include <../libs/json.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <numeric> // Include this for std::accumulate

// Declare counts vector outside the function to maintain state
std::vector<int> counts(10, 0);
std::vector<int> powerPoints(60, 0); // To store power points for levels 1-60
int item_current_idx = 0;
int initialSkillPoints = 85; // Default initial skill points

void IncrementCount(int index) {
    if (counts[index] < 5) counts[index]++;
}

void DecrementCount(int index) {
    if (counts[index] > 0) counts[index]--;
}

void LoadTrainerData() {
    std::ifstream file("trainerdata.json");
    nlohmann::json jsonData;
    file >> jsonData;

    std::string classType = "32"; // Default to "32" for non-mage classes
    initialSkillPoints = 85; // Default initial skill points for non-mage classes

    if (item_current_idx == 2 || item_current_idx == 3) {
        classType = "80"; // Use "80" for mage classes
        initialSkillPoints = 93; // Initial skill points for mage classes
    }

    powerPoints = jsonData["points"]["power"][classType].get<std::vector<int>>();
}

void ShowSkilltrainer(bool &show_Skilltrainer_window, LPDIRECT3DDEVICE9 device) {
    static LPDIRECT3DTEXTURE9 textures[10] = { nullptr };

    if (textures[0] == nullptr) {
        textures[0] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Ensaring_arrow);
        textures[1] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Death_sentence);
        textures[2] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Enemy_surveillance);
        textures[3] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Wild_spirit);
        textures[4] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Camouflage);
        textures[5] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Inherited_lightness);
        textures[6] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Cold_Blood);
        textures[7] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Reveal);
        textures[8] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Confuse);
        textures[9] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Stalker_surroundings);
    }

    if (show_Skilltrainer_window) {
        static const char* items[] = { "Hunter", "Marksman", "Conjurer", "Warlock", "Barbarian", "Knight" };
        static int item_current_idx = 0; 
        static int selected_level = 0;

        ImGui::Text("Class:");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##ClassCombo", items[item_current_idx])) {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected)) {
                    item_current_idx = n;
                    LoadTrainerData(); // Load data based on selected class
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        int totalPointsUsed = std::accumulate(counts.begin(), counts.end(), 0);
        int remainingPoints = initialSkillPoints - totalPointsUsed;
        ImGui::Text("Power Points: %d", remainingPoints);

        ImGui::Text("Level:");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##LevelCombo", std::to_string(selected_level + 1).c_str())) {
            for (int n = 0; n < 60; n++) {
                const bool is_selected = (selected_level == n);
                if (ImGui::Selectable(std::to_string(n + 1).c_str(), is_selected)) {
                    selected_level = n;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        switch (item_current_idx) {
            case 0:
                for (int i = 0; i < 10; ++i) {
                    ImGui::Text("%d", counts[i]); // Display the count
                    ImGui::SameLine();
                    ImGui::Image((void*)textures[i], ImVec2(29, 28));
                    ImGui::SameLine();
                    if (ImGui::Button(("+##" + std::to_string(i)).c_str())) {
                        IncrementCount(i); // Call IncrementCount function
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(("-##" + std::to_string(i)).c_str())) {
                        DecrementCount(i); // Call DecrementCount function
                    }
                }
                break;
            case 1:
                ImGui::Text("2");    
                break;
            case 2:
                ImGui::Text("3");
                break;
            case 3:
                ImGui::Text("4");
                break;
            case 4:
                ImGui::Text("5");
                break;
            case 5:
                ImGui::Text("6");
                break;
            default:
                ImGui::Text("Unknown");
                break;
        }
    }
}