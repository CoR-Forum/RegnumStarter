#include "SkillTrainer.h"
#include "../libs/imgui/imgui.h"
#include "../ui/WindowStates.h" // Include the header file where the window state variables are declared
#include "../libs/ImageLoader/ImageLoader.h"
#include <map>


void ShowSkilltrainer(bool& show_Skilltrainer_window, ImVec4 textColor) {
    if (show_Skilltrainer_window) {
        
        static const char* items[] = { "Hunter", "Marksman", "Conjurer", "Warlock", "Barbarian", "Knight" };
        static int item_current_idx = 0; 
        
        static std::map<int, LPDIRECT3DTEXTURE9> textures;
        if (textures.empty()) {
            textures[0] = LoadTextureFromResource(g_pd3dDevice, IDR_Skilltrainer_ClassH_Ensaring_arrow);
        }

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