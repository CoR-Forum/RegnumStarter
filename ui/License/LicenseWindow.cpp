#include "LicenseWindow.h"
#include "../../libs/imgui/imgui.h"
#include "../../includes/API/ApiHandler.h"
#include "../../includes/API/ApiHandler.h"

void ShowLicenseWindow(bool& show_license_window) {
    if (show_license_window) {
        static char licenseKey[128] = "";

        ImGui::Text("Activate a new license key");

        // Display the input text field for the license key
        ImGui::InputTextWithHint("##License Key", "License Key", licenseKey, IM_ARRAYSIZE(licenseKey));

        // Display the submit button
        if (ImGui::Button("Activate License")) {
            try {
                ActivateLicense(licenseKey);
                ImGui::Text("License activated successfully!");
            } catch (const std::exception& e) {
                Log("Failed to activate license: " + std::string(e.what()));
                ImGui::Text("Failed to activate license: %s", e.what());
            }
        }
        ImGui::Separator();
        // License information from license_runtime_end and license_features
        ImGui::Text("License Expiry: %s", license_runtime_end.c_str());
    }
}