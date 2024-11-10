#include "PlayerWindow.h"
#include "../../includes/Utils.h" // Assuming ReadMemoryValues and IsProcessOpen are defined here
#include "../../includes/process/process.h"
extern std::string sylentx_status;

static float CharValue = 0.9700000286f; // Default small Nordo Value
static bool prevCharState = false; // Track previous state of the checkbox
bool disableCheckboxes = (sylentx_status == "Detected");

void ShowPlayerWindow(bool& show_player_window, bool& optionCharacter) {
    if (show_player_window) {
        /*if (IsProcessOpen("ROClientGame.exe")) {
            std::vector<float> values = ReadMemoryValues({"posx", "posy", "posz"});
            if (values.size() == 3) {
                ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", values[0], values[1], values[2]);
            } else {
                ImGui::Text("Failed to read position values.");
            }
        } */
        ImGui::Spacing();
        ImGui::SeparatorText("TrollOptions");
        ImGui::Spacing();


        ImGui::BeginDisabled(disableCheckboxes);
        ImGui::Checkbox("Character Height", &optionCharacter);
        if (optionCharacter) {
            ImGui::SameLine();
            if (ImGui::SliderFloat("##Character Height", &CharValue, 0.4f, 6.0f)) { // Adjust the range as needed
                MemoryManipulation("ch", CharValue);
                MemoryManipulation("cl", CharValue);
                MemoryManipulation("cb", CharValue);
            }
        } else if (prevCharState) {
            // Reset Value if unchecked
            CharValue = 0.9700000286f;
            MemoryManipulation("cl", CharValue);
            MemoryManipulation("cb", CharValue);
            MemoryManipulation("ch", CharValue);
        }

        prevCharState = optionCharacter; // Update previous state
        ImGui::EndDisabled();
    }
}
