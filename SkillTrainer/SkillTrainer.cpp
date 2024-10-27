#include "SkillTrainer.h"
#include <map>
#include "../ui/WindowStates.h"
#include "../libs/ImageLoader/ImageLoader.h"


std::vector<int> counts(10, 0);

void IncrementCount(int index) {
    if (counts[index] < 5) counts[index]++;
}

void DecrementCount(int index) {
    if (counts[index] > 0) counts[index]--;
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

        if (ImGui::BeginCombo("Class:", items[item_current_idx])) {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_current_idx = n;

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