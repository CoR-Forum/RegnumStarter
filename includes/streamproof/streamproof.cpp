#include "streamproof.h"
#include <string>
#include <sstream>

void SetWindowCaptureExclusion(HWND hwnd, bool exclude)
{
    // Debug log for function entry
    {
        std::ostringstream oss;
        oss << "SetWindowCaptureExclusion called with hwnd: " << hwnd << ", exclude: " << exclude;
        LogDebug(oss.str());
    }

    // Set the window display affinity to exclude from capture
    BOOL result = SetWindowDisplayAffinity(hwnd, exclude ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
    {
        std::ostringstream oss;
        oss << "SetWindowDisplayAffinity result: " << result;
        LogDebug(oss.str());
    }

    // Hide the taskbar icon if exclude is true
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    {
        std::ostringstream oss;
        oss << "Initial window style: " << std::hex << style;
        LogDebug(oss.str());
    }

    if (exclude)
    {
        style |= WS_EX_TOOLWINDOW; // Hide from taskbar
        style &= ~WS_EX_APPWINDOW; // Ensure it is not shown in the taskbar
        {
            std::ostringstream oss;
            oss << "Exclusion enabled, updated style: " << std::hex << style;
            LogDebug(oss.str());
        }
    }
    else
    {
        style &= ~WS_EX_TOOLWINDOW; // Show in taskbar
        style |= WS_EX_APPWINDOW; // Ensure it is shown in the taskbar
        {
            std::ostringstream oss;
            oss << "Exclusion disabled, updated style: " << std::hex << style;
            LogDebug(oss.str());
        }
    }
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
    {
        std::ostringstream oss;
        oss << "SetWindowLongPtr called with updated style: " << std::hex << style;
        LogDebug(oss.str());
    }

    // Force the window to refresh its style
    BOOL posResult = SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    {
        std::ostringstream oss;
        oss << "SetWindowPos result: " << posResult;
        LogDebug(oss.str());
    }
}