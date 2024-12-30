#pragma once

#include "../../includes/Utils.h"
#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imfilebrowser.h"
#include "../../includes/Utils.h"
#include "../../includes/API/ApiHandler.h"
#include <vector>
#include <string>

// Function to show the RegnumStarter UI
void ShowRegnumSettings(bool& show_RegnumSettings);

// Function to show the Regnum Accounts window
void ShowRegnumAccounts(bool& show_RegnumAccounts);

// Function to run the Regnum Online client game
void runRoClientGame(const std::string& regnumLoginUser, const std::string& regnumLoginPassword);

// Define the checkbox states
bool enableMusic = true;
bool enableSoundEffects = true;
bool IgnoreServerTime = true;
float serverTime = 12.014381f; 
bool showLoadingScreen = true;
bool showIntro = true;
float soundVolume = 0.5f;
std::string envWeather = "clear";