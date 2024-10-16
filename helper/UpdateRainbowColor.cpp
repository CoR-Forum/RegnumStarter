#include "UpdateRainbowColor.h"

// UI function to update the rainbow color
void UpdateRainbowColor(float speed) {
    float time = ImGui::GetTime() * speed;
    textColor.x = (sin(time) * 0.5f) + 0.5f;
    textColor.y = (sin(time + 2.0f) * 0.5f) + 0.5f;
    textColor.z = (sin(time + 4.0f) * 0.5f) + 0.5f;
}