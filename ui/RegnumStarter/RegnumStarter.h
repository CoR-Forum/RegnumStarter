#pragma once

#include "../../includes/Utils.h"
#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imfilebrowser.h"
#include "../../includes/Utils.h"
#include "../../includes/API/ApiHandler.h"
#include <vector>
#include <string>

// Function to show the RegnumStarter UI
void ShowRegnumStarter(bool& show_RegnumStarter);

// Function to run the Regnum Online client game
void runRoClientGame(const std::string& regnumLoginUser, const std::string& regnumLoginPassword);

// Define the checkbox states
bool enableMusic = true;
bool enableSoundEffects = true;
bool showLoadingScreen = true;
bool showIntro = true;
float soundVolume = 0.5f;