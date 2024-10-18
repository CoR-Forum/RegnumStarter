#include "LoadingScreen.h"
#include "../../libs/imgui/imgui.h"
#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>

static float progress = 0.0f; // Move progress to global scope

// Function to display the loading screen
void ShowLoadingScreen(bool& show_loading_screen, const std::string& statusMessage, bool& loginSuccess) {
    static bool loadingWindowIsOpen = true;
    static bool showResult = false;
    static bool closeWindow = false;

    // Set the size of the ImGui window
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Always);

    // Begin the ImGui window
    ImGui::Begin("Loading", &loadingWindowIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // If the window is closed, save settings and quit
    if (!loadingWindowIsOpen) {
        SaveSettings();
        PostQuitMessage(0);
    }

    // Center the loading animation and status message
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 textSize = ImGui::CalcTextSize("Loading...");
    ImVec2 statusTextSize = ImGui::CalcTextSize(statusMessage.c_str());
    ImVec2 resultTextSize = ImGui::CalcTextSize(loginSuccess ? "Login successful!" : "Login failed. Please try again.");

    // Center the loading text vertically and horizontally
    ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f - 20));
    ImGui::Text("Loading...");
    ImGui::SameLine();
    static const char* loadingFrames[] = { "|", "/", "-", "\\" };
    static int frame = 0;
    frame = (frame + 1) % 4;
    ImGui::TextUnformatted(loadingFrames[frame]);

    // Center the status message text
    ImGui::SetCursorPos(ImVec2((windowSize.x - statusTextSize.x) * 0.5f, (windowSize.y - statusTextSize.y) * 0.5f));
    ImGui::Text("%s", statusMessage.c_str());

    // Display the progress bar
    ImGui::SetCursorPos(ImVec2((windowSize.x - 200) * 0.5f, (windowSize.y - 20) * 0.5f + 40));
    ImGui::ProgressBar(progress, ImVec2(200, 20));

    // If login is successful or failed, display the result
    if (showResult) {
        ImGui::SetCursorPos(ImVec2((windowSize.x - resultTextSize.x) * 0.5f, (windowSize.y - resultTextSize.y) * 0.5f + 60));
        if (loginSuccess) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green color
            ImGui::Text("Login successful!");
            ImGui::PopStyleColor();
            closeWindow = true;
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color
            ImGui::Text("Login failed. Please try again.");
            ImGui::PopStyleColor();
            closeWindow = true;
        }
    }

    // Close the window after showing the result
    if (closeWindow) {
        show_loading_screen = false;
        loadingWindowIsOpen = false;
        showResult = false; // Reset showResult for the next login attempt
        closeWindow = false; // Reset closeWindow for the next login attempt
        progress = 0.0f; // Reset progress for the next login attempt
    }

    // End the ImGui window
    ImGui::End();
}

// Function to update the progress bar
void UpdateProgressBar(float value) {
    progress = value;
}