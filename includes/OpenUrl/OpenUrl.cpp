// includes/OpenUrl/OpenUrl.cpp
#include <shellapi.h> // Include for ShellExecute
#include "OpenUrl.h"

// Function to open a URL in the default web browser
void OpenURL(const char* url) {
    ShellExecute(0, 0, url, 0, 0, SW_SHOW);
}