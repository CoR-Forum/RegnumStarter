// ui/helper/ShowHelpMarker.cpp
#include "ShowHelpMarker.h"
#include "../../libs/imgui/imgui.h"

void ShowHelpMarker(const char* desc)
{   
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.098f, 0.098f, 0.902f, 1.0f)); // Color #1919e6
    ImGui::TextDisabled("(?)");
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}