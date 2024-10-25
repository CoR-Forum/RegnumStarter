#include "CreditsWindow.h"

void ShowCreditsWindow(bool& show_info_window) {
    if (show_info_window) {
        ImGui::Text("This software is provided as-is without any warranty. Use at your own risk.");
        ImGui::Text("Made with hate in Germany by AdrianWho, Manu and Francis");
        ImGui::Text("Special thanks to the Champions of Regnum community for their support and feedback.");
        ImGui::Text("Big shoutout to Adrian Lastres. You're the best!");
    }
}