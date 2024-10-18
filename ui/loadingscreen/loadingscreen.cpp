#include "LoadingScreen.h"
#include "../../libs/imgui/imgui.h"
#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>

// Function to display the loading screen
void ShowLoadingScreen(bool& show_loading_screen, const std::string& statusMessage, bool& loginSuccess) {
    static bool loadingWindowIsOpen = true;
    static auto startTime = std::chrono::steady_clock::now();
    static bool showResult = false;
    static bool closeWindow = false;
    static auto resultTime = std::chrono::steady_clock::now();

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

    // Check if enough time has passed to show the result
    auto currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count() >= 2) {
        showResult = true;
        resultTime = std::chrono::steady_clock::now();
    }

    // If login is successful or failed, display the result after the delay
    if (showResult) {
        ImGui::SetCursorPos(ImVec2((windowSize.x - resultTextSize.x) * 0.5f, (windowSize.y - resultTextSize.y) * 0.5f + 20));
        if (loginSuccess) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green color
            ImGui::Text("Login successful!");
            ImGui::PopStyleColor();
            if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - resultTime).count() >= 2) {
                closeWindow = true;
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color
            ImGui::Text("Login failed. Please try again.");
            ImGui::PopStyleColor();
            if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - resultTime).count() >= 4) {
                closeWindow = true;
            }
        }
    }

    // Close the window after showing the result
    if (closeWindow) {
        show_loading_screen = false;
        loadingWindowIsOpen = false;
    }

    // End the ImGui window
    ImGui::End();
}