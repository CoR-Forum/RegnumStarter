#include "SkillTrainer.h"
#include <map>
#include "../ui/WindowStates.h"
#include "../libs/ImageLoader/ImageLoader.h"

void ShowSkilltrainer(bool &show_Skilltrainer_window, LPDIRECT3DDEVICE9 device) {
    static LPDIRECT3DTEXTURE9 textures[6] = { nullptr };

    if (textures[0] == nullptr) {
        textures[0] = LoadTextureFromResource(device, IDR_Skilltrainer_ClassH_Acrobatic);
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
                ImGui::Text("1");
                ImGui::Image((void*)textures[0], ImVec2(32, 32)); 
                break;
            case 1:
                ImGui::Text("2");
                //ImGui::Image((void*)textures[1], ImVec2(32, 32)); 
                break;
            case 2:
                ImGui::Text("3");
                //ImGui::Image((void*)textures[2], ImVec2(32, 32)); 
                break;
            case 3:
                ImGui::Text("4");
                //ImGui::Image((void*)textures[3], ImVec2(32, 32)); 
                break;
            case 4:
                ImGui::Text("5");
                //ImGui::Image((void*)textures[4], ImVec2(32, 32)); 
                break;
            case 5:
                ImGui::Text("6");
                //ImGui::Image((void*)textures[5], ImVec2(32, 32)); 
                break;
            default:
                ImGui::Text("Unknown");
                break;
        }
    }
}