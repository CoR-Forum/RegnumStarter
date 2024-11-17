#pragma once

#include "../../includes/Utils.h"
#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui/imfilebrowser.h"
#include "../../includes/API/ApiHandler.h"
#include <vector>
#include <string>

// Function to show the RegnumStarter UI
void ShowRegnumStarter(bool& show_RegnumStarter);

// Function to run the Regnum Online client game
void runRoClientGame(const std::string& regnumLoginUser, const std::string& regnumLoginPassword);

// Define the checkbox states
namespace RegnumSettings {
    inline bool enableMusic = true;
    inline bool enableSoundEffects = true;
    inline bool showLoadingScreen = true;
    inline bool showIntro = true;
    inline float soundVolume = 0.5f;
}