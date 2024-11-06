#include "SkillTrainer.h"
#include <map>
#include "../ui/WindowStates.h"
#include "../libs/ImageLoader/ImageLoader.h"
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <windows.h> // For getting the desktop path

// Declare counts vector outside the function to maintain state
std::vector<int> counts(60, 0); // Increase the size to 60
std::vector<int> powerPoints(60, 0); // To store power points for levels 1-60
int item_current_idx = 0;

// Initialize the ArcherWarrior array with the provided values
int ArcherWarrior[60] = {
    4, 4, 4, 4, 5, 6, 7, 8, 9, 9,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 43,
    45, 47, 49, 51, 53, 55, 57, 59, 61, 65,
    67, 69, 71, 73, 75, 77, 79, 81, 83, 85
};

// Initialize the ConjurerWarlock array with the provided values
int ConjurerWarlock[60] = {
    4, 4, 4, 4, 5, 6, 7, 8, 9, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 44,
    46, 48, 50, 53, 55, 58, 61, 64, 67, 73,
    75, 77, 79, 81, 83, 85, 87, 89, 91, 93
};

void IncrementCount(int index) {
    if (counts[index] < 5) counts[index]++;
}

void DecrementCount(int index) {
    if (counts[index] > 0) counts[index]--;
}

std::string GetDesktopPath() {
    char path[MAX_PATH];
    if (SHGetSpecialFolderPathA(NULL, path, CSIDL_DESKTOP, FALSE)) {
        return std::string(path);
    }
    return "";
}

void SaveConfig() {
    std::string className = "Hunter"; // Hunter (class that i used as example)
    int level = 60; // level (still needs enhancements)

    std::string configName = className + std::to_string(level);
    std::string treeConfig;
    for (int i = 0; i < 60; i += 10) {
        for (int j = 0; j < 10; ++j) {
            treeConfig += std::to_string(counts[i + j]);
        }
        if (i < 50) {
            treeConfig += ",";
        }
    }

    std::string configContent = configName + "::" + treeConfig;

    std::string desktopPath = GetDesktopPath();
    std::string filePath = desktopPath + "\\Sylent-XConfig.txt";
    int fileIndex = 1;

    while (std::ifstream(filePath)) {
        filePath = desktopPath + "\\Sylent-XConfig" + std::to_string(fileIndex) + ".txt";
        fileIndex++;
    }

    std::ofstream file(filePath);
    file << configContent;
    file.close();
}

void ShowSkilltrainer(bool &show_Skilltrainer_window, LPDIRECT3DDEVICE9 device) {
    static LPDIRECT3DTEXTURE9 textures[60] = { nullptr }; // Increase the size to 60

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
        textures[10] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Natural_dominance);
        textures[11] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Heal_pet);
        textures[12] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Wild_defence);
        textures[13] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Prominent_sight);
        textures[14] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Training);
        textures[15] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Revive_pet);
        textures[16] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Bestial_Wrath);
        textures[17] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Natural_armor);
        textures[18] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Natures_fury);
        textures[19] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Skin_of_the_beast);
        textures[20] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Dual_shot);
        textures[21] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Rapid_shot);
        textures[22] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Grounding_arrow);
        textures[23] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Lightness);
        textures[24] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Power_meditation);
        textures[25] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Adaptability);
        textures[26] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Tear_apart);
        textures[27] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Duelist);
        textures[28] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Power_hinder);
        textures[29] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Repeatition_shot);
        textures[30] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Shield_Piercing);
        textures[31] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Parabolic_shot);
        textures[32] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Maneuver);
        textures[33] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Eagles_eye);
        textures[34] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Point_shot);
        textures[35] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Omnipresent);
        textures[36] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Break_apart);
        textures[37] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Specialist);
        textures[38] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Obfuscate);
        textures[39] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Projectile_Rain);
        textures[40] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Ambush);
        textures[41] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Sudden_strike);
        textures[42] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Stunning_fist);
        textures[43] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Retaliation);
        textures[44] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Sticky_touch);
        textures[45] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Dirty_fighting);
        textures[46] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Finesse);
        textures[47] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Sentinel);
        textures[48] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Distracting_shot);
        textures[49] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Caltrops_arrow);
        textures[50] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Dodge);
        textures[51] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Mobility);
        textures[52] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Evasive_tactics);
        textures[53] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Cat_reflexes);
        textures[54] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Acrobatic);
        textures[55] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Spell_elude);
        textures[56] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Escapist);
        textures[57] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Wits);
        textures[58] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Low_profile);
        textures[59] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Son_of_the_wind);
    }

    if (show_Skilltrainer_window) {
        static const char* items[] = { "Hunter", "Marksman", "Conjurer", "Warlock", "Barbarian", "Knight" };
        static int selected_level = 59; // Set default level to 60 (index 59)

        ImGui::Text("Class:");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##ClassCombo", items[item_current_idx])) {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected)) {
                    item_current_idx = n;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

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

        int totalPointsUsed = std::accumulate(counts.begin(), counts.end(), 0);
        int remainingPoints;

        // Use the appropriate array based on the selected class
        if (item_current_idx == 0 || item_current_idx == 1 || item_current_idx == 4 || item_current_idx == 5) {
            remainingPoints = ArcherWarrior[selected_level] - totalPointsUsed;
        } else {
            remainingPoints = ConjurerWarlock[selected_level] - totalPointsUsed;
        }

        // Display the remaining power points
        ImGui::Text("Remaining Power Points: %d", remainingPoints);

        switch (item_current_idx) {
            case 0: // Hunter
                ImGui::BeginGroup(); // First tree
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
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup(); // Second tree
                for (int i = 10; i < 20; ++i) {
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
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup(); // Third tree
                for (int i = 20; i < 30; ++i) {
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
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup(); // Fourth tree
                for (int i = 30; i < 40; ++i) {
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
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup(); // Fifth tree
                for (int i = 40; i < 50; ++i) {
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
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup(); // Sixth tree
                for (int i = 50; i < 60; ++i) {
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
                ImGui::EndGroup();
                break;
            case 1: // Marksman
                ImGui::Text("Marksman");
                break;
            case 2: // Conjurer
                ImGui::Text("Conjurer");
                break;
            case 3: // Warlock
                ImGui::Text("Warlock");
                break;
            case 4: // Barbarian
                ImGui::Text("Barbarian");
                break;
            case 5: // Knight
                ImGui::Text("Knight");
                break;
            default:
                ImGui::Text("Unknown");
                break;
        }

        if (ImGui::Button("Save Config")) {
            SaveConfig();
        }
    }
}