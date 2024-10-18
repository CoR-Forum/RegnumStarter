#pragma once

#ifndef LOADING_SCREEN_H
#define LOADING_SCREEN_H

#include <string>

// Function to display the loading screen
void ShowLoadingScreen(bool& show_loading_screen, const std::string& statusMessage, bool& loginSuccess);

// Function to save settings (externally defined)
extern void SaveSettings();

#endif // LOADING_SCREEN_H