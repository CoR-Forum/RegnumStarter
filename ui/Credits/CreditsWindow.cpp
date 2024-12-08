#include "CreditsWindow.h"

void ShowCreditsWindow(bool& show_info_window) {
    if (show_info_window) {
        ImGui::Text("This software is provided as-is without any warranty. Use at your own risk.");
        ImGui::Text("We are not responsible for any damages or Regnum account bans caused by the use of this software.");
        ImGui::Text("Although gettig banned for using this software is highly unlikely, it is still a possibility.");
        ImGui::Text("Please use this software responsibly and respect the game's terms of service.");
        ImGui::Spacing();
        ImGui::Text("Special thanks to the Champions of Regnum community for their support and feedback.");
        ImGui::Text("Contributions are highly appreciated. Feel free to get in touch with us on Discord.");
        ImGui::Spacing();
        ImGui::Text("Big shoutout to Adrian Lastres. You're the best and we love you! Thank you for keeping Regnum running!");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "<3");
        ImGui::Spacing();
        ImGui::Text("This software is not affiliated with Nimble Giant Entertainment or Champions of Regnum.");
        ImGui::Text("All trademarks are property of their respective owners.");
        ImGui::Spacing();
        ImGui::Text("Authors: Francis, AdrianWho? and manu");
    }
}