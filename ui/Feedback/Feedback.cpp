#include "Feedback.h"
#include "../../libs/imgui/imgui.h"
#include "../../includes/API/ApiHandler.h"
#include <string>
#include <exception>


void ShowFeedbackWindow(bool& show_feedback_window) {
    if (show_feedback_window) {
        static int feedbackType = 0;
        static bool feedback_includeLogfile = true;
        const char* feedbackTypes[] = { "Suggestion", "Bug Report", "Other" };
        static char feedbackText[1024] = "";
        static std::string feedbackMessage = "";

        ImGui::Combo("Type", &feedbackType, feedbackTypes, IM_ARRAYSIZE(feedbackTypes));

        ImGui::SameLine();
        ImGui::Checkbox("Include Log File", &feedback_includeLogfile);

        ImGui::InputTextMultiline("Feedback", feedbackText, IM_ARRAYSIZE(feedbackText), ImVec2(480, ImGui::GetTextLineHeight() * 10));

        if (ImGui::Button("Submit")) {
            try {
                SendFeedback(feedbackTypes[feedbackType], feedbackText, feedback_includeLogfile);
                feedbackMessage = "Feedback sent successfully!";
                feedbackText[0] = '\0'; // Clear the feedback text
            } catch (const std::exception& e) {
                feedbackMessage = "Failed to send feedback: " + std::string(e.what());
            }
        }

        if (!feedbackMessage.empty()) {
            ImGui::Text("%s", feedbackMessage.c_str());
        }
    }
}