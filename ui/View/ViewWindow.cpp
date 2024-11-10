#include "ViewWindow.h"
#include "../../includes/Utils.h" // Assuming MemoryManipulation is declared here
#include "../../ui/helper/Markers/LicenseMarker.h"
#include <iostream> // For debugging

extern std::string sylentx_status;

// Function to get the key name from the virtual key code
std::string GetKeyName(int virtualKey) {
    if (virtualKey < 0x08 || virtualKey > 0xFF) {
        std::cout << "Invalid virtualKey: " << virtualKey << std::endl;
        return "Unknown";
    }

    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    if (scanCode == 0) {
        std::cout << "Failed to map virtualKey: " << virtualKey << std::endl;
        return "Unknown";
    }

    char keyName[128];
    int result = GetKeyNameText(scanCode << 16, keyName, sizeof(keyName));
    
    // Debugging information
    std::cout << "virtualKey: " << virtualKey << ", scanCode: " << scanCode << ", result: " << result << std::endl;

    if (result > 0) {
        return std::string(keyName);
    }
    return "Unknown";
}

void ShowViewWindow(bool& show_view_window, bool& optionZoom, bool& optionFov, bool& featureFov, bool& waitingForHotkey, int& userDefinedHotkey) {
    if (show_view_window) {
        static float zoomValue = 15.0f; // Default zoom value
        static bool prevZoomState = false; // Track previous state of the checkbox
        bool disableCheckboxes = (sylentx_status == "Detected");

        ImGui::BeginDisabled(disableCheckboxes);
        ImGui::Checkbox("Enable Zoom", &optionZoom);
        if (optionZoom) {
            ImGui::SameLine();
            if (ImGui::SliderFloat("Zoom", &zoomValue, 15.0f, 60.0f)) { // Adjust the range as needed
                MemoryManipulation("zoom", zoomValue);
            }
        } else if (prevZoomState) {
            // Reset zoom value to 15.0f when checkbox is unchecked
            zoomValue = 15.0f;
            MemoryManipulation("zoom", zoomValue);
        }

        prevZoomState = optionZoom; // Update previous state
        ImGui::EndDisabled();

        // Check if the checkbox is checked
        ImGui::BeginDisabled(disableCheckboxes || !featureFov);
        if (ImGui::Checkbox("Field of View", &optionFov)) {
            if (!optionFov) {
                // If the checkbox is unchecked, reset the FOV value
                MemoryManipulation("fov", 0.01745329238f);
            }
        }
        ImGui::EndDisabled();
        ImGui::SameLine();

        // Only allow users to set the hotkey if the checkbox is checked
        if (optionFov) {
            std::string buttonLabel;
            if (waitingForHotkey) {
                buttonLabel = "Press any key...";
            } else if (userDefinedHotkey == 0) {
                buttonLabel = "Set Hotkey";
            } else {
                buttonLabel = "Hotkey: " + GetKeyName(userDefinedHotkey);
            }

            if (ImGui::Button(buttonLabel.c_str())) {
                waitingForHotkey = true;
            }

            if (waitingForHotkey) {
                for (int key = 0x08; key <= 0xFF; key++) {
                    if (GetAsyncKeyState(key) & 0x8000) {
                        userDefinedHotkey = key;
                        std::cout << "Hotkey set to: " << userDefinedHotkey << std::endl; // Debugging information
                        waitingForHotkey = false;
                        break;
                    }
                }
            }
        }

        if (!featureFov) {
            ImGui::SameLine();
            ShowLicenseMarker();
        }
    }
}