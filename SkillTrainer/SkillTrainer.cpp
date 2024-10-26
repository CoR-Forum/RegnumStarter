#include "SkillTrainer.h"
#include "../../libs/imgui/imgui.h"

void ShowSkilltrainer(bool* show_Skilltrainer_window) {
    if (show_Skilltrainer_window && *show_Skilltrainer_window) {
        ImGui::Begin("Skill Trainer", show_Skilltrainer_window);

        static const char* items[] = { "Hunter", "Marksman", "Conjurer", "Warlock", "Barbarian", "Knight" };
        static int item_current_idx = 0; // Here we store our selection data as an index.

        if (ImGui::BeginCombo("Examples", items[item_current_idx])) {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_current_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::End();
    }
}