#include "streamproof.h"

void SetWindowCaptureExclusion(HWND hwnd, bool exclude)
{
    // Set the window display affinity to exclude from capture
    SetWindowDisplayAffinity(hwnd, exclude ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);

    // Hide the taskbar icon if exclude is true
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exclude)
    {
        style |= WS_EX_TOOLWINDOW; // Hide from taskbar
        style &= ~WS_EX_APPWINDOW; // Ensure it is not shown in the taskbar
    }
    else
    {
        style &= ~WS_EX_TOOLWINDOW; // Show in taskbar
        style |= WS_EX_APPWINDOW; // Ensure it is shown in the taskbar
    }
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);

    // Force the window to refresh its style
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}