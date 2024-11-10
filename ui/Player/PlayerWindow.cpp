#include "PlayerWindow.h"
#include "../../includes/Utils.h" // Assuming ReadMemoryValues and IsProcessOpen are defined here
#include "../../includes/process/process.h"
#include <iostream>
#include <fstream>

// Redirect std::cout and std::cerr to a null stream to disable console output
std::ofstream null_stream;
std::streambuf* cout_buf = std::cout.rdbuf(null_stream.rdbuf());
std::streambuf* cerr_buf = std::cerr.rdbuf(null_stream.rdbuf());

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
    // Restore std::cout and std::cerr to their original state
    std::cout.rdbuf(cout_buf);
    std::cerr.rdbuf(cerr_buf);
}