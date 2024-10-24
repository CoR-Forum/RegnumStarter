#pragma once

#include "includes/Utils.h"
#include <stdexcept>
#include <sstream>
#include <regex>

extern HWND hwnd;

// global variables for user login and password
std::string login;
std::string password;

// global variables for sylent-x
extern std::string sylentx_status;

// global variables for user license information
std::string license_runtime_end;
std::string license_features;

// global variables for user licensed features
bool featureZoom;
bool featureGravity;
bool featureMoonjump;
bool featureMoonwalk;
bool featureFov;
bool featureSpeedhack;
bool featureFreecam;
bool featureFastfly;
bool featureFakelag;

// global variables for (checkbox) options
bool optionGravity = false;
bool optionMoonjump = false;
bool optionZoom = false;
bool optionFreecam = false;
bool optionMoonwalk = false;
bool optionFov = false;
bool optionFastFly = false;
bool optionSpeedHack = false;
bool optionFakelag = false;

// global variables for settings (from user account via API)
float setting_fontSize = 14.0f;
bool setting_enableRainbow = false;
float setting_rainbowSpeed = 0.1f;
bool setting_excludeFromCapture = false;
std::string setting_regnumInstallPath;
bool setting_log_debug = true;
int setting_log_maxMessages = 10;
bool isAdmin = false;

// global variables for license key generation
std::string generated_license_key;

std::string GetAllLicensesRawJson;
std::string GetAllUsersRawJson;
std::vector<Pointer> g_pointers;
std::vector<std::string> g_chatMessages; 

void ModifyGlobalSettings(const std::string& settingName, const std::string& settingValue);