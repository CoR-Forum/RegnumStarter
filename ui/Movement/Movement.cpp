#include "Movement.h"
#include "../../libs/imgui/imgui.h"
#include "../../includes/API/ApiHandler.h"
#include "../../ui/helper/Markers/HelpMarker.h"
#include "../../ui/helper/Markers/LicenseMarker.h"
#include "../../includes/Utils.h"
#include <thread>

void ShowMovementWindow(bool& show_movement_window) {
    if (!show_movement_window) return;

    ImGui::BeginDisabled(!featureSpeedhack);
    if (ImGui::Checkbox("SpeedHack", &optionSpeedHack)) {
        float newValue = optionSpeedHack ? 5.6f : 4.8f;
        MemoryManipulation("speedhack", newValue);
    }
    if (featureSpeedhack) {
        ImGui::SameLine();
        ShowHelpMarker("Use at own risk");
    }
    ImGui::EndDisabled();
    if (!featureSpeedhack) {
        ImGui::SameLine();
        ShowLicenseMarker();
    }

    ImGui::BeginDisabled(!featureGravity);
    if (ImGui::Checkbox("Flyhack", &optionGravity)) {
        MemoryManipulation("gravity");
    }
    ImGui::EndDisabled();
    if (!featureGravity) {
        ImGui::SameLine();
        ShowLicenseMarker();
    }

    static float moonjumpValue = 4.0f; // Default moonjump value
    static bool prevjumpState = false; // Track previous state of the checkbox
    ImGui::BeginDisabled(!featureMoonjump);
    if (ImGui::Checkbox("Moonjump", &optionMoonjump)) {
        if (optionMoonjump) {
            prevjumpState = true;
        } else if (prevjumpState) {
            // Reset zoom value to 4.0f when checkbox is unchecked
            moonjumpValue = 4.0f;
            MemoryManipulation("moonjump", moonjumpValue);
            prevjumpState = false;
        }
    }
    if (optionMoonjump) {
        ImGui::SameLine();
        if (ImGui::SliderFloat("##MoonjumpSlider", &moonjumpValue, 0.3f, 4.0f)) { // Adjust the range as needed
            MemoryManipulation("moonjump", moonjumpValue);
        }
        ImGui::SameLine();
        ShowHelpMarker("We recommend value 1.00");
    }
    ImGui::EndDisabled();
    if (!featureMoonjump) {
        ImGui::SameLine();
        ShowLicenseMarker();
    }

    ImGui::BeginDisabled(!featureMoonwalk);
    if (ImGui::Checkbox("Moonwalk", &optionMoonwalk)) {
        if (optionMoonwalk) {
            float newValue = 9.219422856E-41f;
            MemoryManipulation("moonwalk", newValue);
            MemoryManipulation("moonwalk", newValue);
            std::thread(UncheckMoonwalkAfterDelay, std::ref(optionMoonwalk)).detach();
        }
    }
    ImGui::EndDisabled();
    if (!featureMoonwalk) {
        ImGui::SameLine();
        ShowLicenseMarker();
    }

    ImGui::BeginDisabled(!featureFakelag);
    if (ImGui::Checkbox("Fakelag", &optionFakelag)) {
        if (optionFakelag) {
            float newValue = 0.0f;
            MemoryManipulation("fakelag", newValue);
            MemoryManipulation("fakelagg", newValue);
            std::thread(UncheckFakelagAfterDelay, std::ref(optionFakelag)).detach();
        }
    }
    ImGui::EndDisabled();
    if (!featureFakelag) {
        ImGui::SameLine();
        ShowLicenseMarker();
    }

    if (isAdmin) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Admin Options:");
        ImGui::Spacing();
        static float fastflyValue = 250.0f; // Default moonjump value
        static bool prevflyState = false; // Track previous state of the checkbox
        ImGui::BeginDisabled(!featureFastfly);
        if (ImGui::Checkbox("FastFly", &optionFastFly)) {
            if (optionFastFly) {
                prevflyState = true;
            } else if (prevflyState) {
                // Reset fly value to 4.8f when checkbox is unchecked
                fastflyValue = 4.8f;
                MemoryManipulation("fastfly", fastflyValue);
                prevflyState = false;
            }
        }
        ImGui::EndDisabled();
        if (optionFastFly) {
            ImGui::SameLine();
            if (ImGui::SliderFloat("##FastFlySlider", &fastflyValue, 4.8f, 250.0f)) { // Adjust the range as needed
                MemoryManipulation("fastfly", fastflyValue);
            }
        }
        ImGui::SameLine();
        if (!featureFastfly) {
            ImGui::SameLine();
            ShowLicenseMarker();
        }
    }
}