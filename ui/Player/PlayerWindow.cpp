#include "PlayerWindow.h"
#include "../../includes/Utils.h" // Assuming ReadMemoryValues and IsProcessOpen are defined here
#include "../../includes/process/process.h"
#include <iostream>
#include <fstream>

// Redirect std::cout to a null stream to disable console output
std::ofstream null_stream;
std::streambuf* cout_buf = std::cout.rdbuf(null_stream.rdbuf());
extern std::string sylentx_status;

static float CharValue = 0.9700000286f; // Default small Nordo Value
static bool prevCharState = false; // Track previous state of the checkbox
bool disableCheckboxes = (sylentx_status == "Detected");

void ShowPlayerWindow(bool& show_player_window, bool& optionCharacter) {
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
    // Restore std::cout to its original state
    std::cout.rdbuf(cout_buf);
}

