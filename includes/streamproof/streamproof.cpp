#include "streamproof.h"

void SetWindowCaptureExclusion(HWND hwnd, bool exclude)
{
    // Debug log for function entry
    DebugLog("SetWindowCaptureExclusion called with hwnd: %p, exclude: %d", hwnd, exclude);

    // Set the window display affinity to exclude from capture
    BOOL result = SetWindowDisplayAffinity(hwnd, exclude ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
    DebugLog("SetWindowDisplayAffinity result: %d", result);

    // Hide the taskbar icon if exclude is true
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    DebugLog("Initial window style: %lx", style);

    if (exclude)
    {
        style |= WS_EX_TOOLWINDOW; // Hide from taskbar
        style &= ~WS_EX_APPWINDOW; // Ensure it is not shown in the taskbar
        DebugLog("Exclusion enabled, updated style: %lx", style);
    }
    else
    {
        style &= ~WS_EX_TOOLWINDOW; // Show in taskbar
        style |= WS_EX_APPWINDOW; // Ensure it is shown in the taskbar
        DebugLog("Exclusion disabled, updated style: %lx", style);
    }
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
    DebugLog("SetWindowLongPtr called with updated style: %lx", style);

    // Force the window to refresh its style
    BOOL posResult = SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    DebugLog("SetWindowPos result: %d", posResult);
}