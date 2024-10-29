#pragma once

#include "../Utils.h"
#include "../../ui/Memory/Memory.h"
#include "../InternetUtils/InternetUtils.cpp"
#include <stdexcept>
#include <sstream>
#include <regex>
#include "../md5/md5.h"

extern HWND hwnd;

// global variables for user login and password
std::string login;
std::string password;

// variable to store current amount of Magnat currency for the user
int magnatCurrency;

// global variables for sylent-x
std::string sylentx_status;

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

// Declare the checkbox states as global variables
extern bool enableMusic;
extern bool enableSoundEffects;
extern bool showLoadingScreen;
extern bool ShowIntro;
extern float soundVolume;

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
bool isAdmin = false;

// global variables for memory pointers and chat messages
std::vector<Pointer> g_pointers;
std::vector<std::string> g_chatMessages;

// global variables for chat input
static char chatInput[256] = "";

struct RegnumAccount {
    int id;
    std::string username;
    std::string password;
    std::string server;
    std::string referrer;
};

struct ServerOption {
    const char* id;
    const char* name;
};

struct ReferrerOption {
    const char* id;
    const char* name;
};

// global variable to store the loaded regnum accounts
std::vector<RegnumAccount> regnumAccounts;

// global functions for chat
void SendChatMessage(const std::string& message);
void CheckChatMessages();

bool Login(const std::string& login, const std::string& password);
void RegisterUser(const std::string& username, const std::string& nickname, const std::string& email, const std::string& password);
void LoadLoginCredentials(HINSTANCE hInstance);
void SaveLoginCredentials(const std::string& login, const std::string& encryptedPassword);
void ActivateLicense(const std::string& licenseKey);
void SendFeedback(const std::string& type, const std::string& feedback, bool feedback_includeLogfile);
void LoadRegnumAccounts();
void SaveRegnumAccount(const std::string& username, const std::string& password, const std::string& server, const std::string& referrer, int id);
void DeleteRegnumAccount(int id);
std::vector<Pointer> InitializePointers();