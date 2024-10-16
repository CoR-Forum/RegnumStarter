// ui/helper/ShowLicenseMarker.cpp
#include "ShowLicenseMarker.h"
#include "../../libs/imgui/imgui.h"

void ShowLicenseMarker()
{   
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.098f, 0.098f, 0.902f, 1.0f)); // Color #1919e6
    ImGui::TextDisabled("(!!)");
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted("This feature is only available in the premium version of the software. Please consider purchasing a license to unlock this feature.");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}