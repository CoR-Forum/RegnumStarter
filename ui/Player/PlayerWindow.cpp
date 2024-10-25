#include "PlayerWindow.h"
#include "../../includes/Utils.h" // Assuming ReadMemoryValues and IsProcessOpen are defined here
#include "../../includes/process/process.h"

void ShowPlayerWindow(bool& show_player_window) {
    if (show_player_window) {
        if (IsProcessOpen("ROClientGame.exe")) {
            std::vector<float> values = ReadMemoryValues({"posx", "posy", "posz"});
            if (values.size() == 3) {
                ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", values[0], values[1], values[2]);
            } else {
                ImGui::Text("Failed to read position values.");
            }
        }
    }
}