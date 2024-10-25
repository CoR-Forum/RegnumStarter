#include "Chat.h"
#include "../../libs/imgui/imgui.h"
#include "../../ui/WindowStates.h"
#include "../../includes/API/ApiHandler.h"
#include <vector>
#include <string>

extern std::vector<std::string> g_chatMessages;
extern char chatInput[256];

void ShowChatWindow(bool& show_chat_window) {
    if (show_chat_window) {
        ImGui::Begin("Chat", &show_chat_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

        // Log display box at the bottom
        ImGui::BeginChild("ChatMessages", ImVec2(550, 200), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        for (const auto& msg : g_chatMessages) {
            ImGui::TextWrapped("%s", msg.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f); // Scroll to the bottom
        }

        ImGui::EndChild();

        ImGui::InputTextWithHint("##ChatInput", "Type your message here...", chatInput, IM_ARRAYSIZE(chatInput));

        ImGui::SameLine();
        
        if (ImGui::Button("Send Message") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
            if (strlen(chatInput) > 0) {
                SendChatMessage(chatInput);
                chatInput[0] = '\0'; // Clear input field
            }
        }
        ImGui::End();
    }
}