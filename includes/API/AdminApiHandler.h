#pragma once

#include "../Utils.h"
#include "../InternetUtils/InternetUtils.h"
#include "../md5/md5.h"
#include <stdexcept>
#include <sstream>
#include <regex>
#include <string>

// Forward declaration
extern HWND hwnd;

// Global variables for license key generation
std::string generated_license_key;
std::string GetAllLicensesRawJson;
std::string GetAllUsersRawJson;

// Function declarations
void ModifyGlobalSettings(const std::string& settingName, const std::string& settingValue);
void GetAllLicenses();
void DisplayUsersTable();
void ToggleUserBan(int userId);
void ToggleUserAdmin(int userId);
void ToggleUserActivation(int userId);
void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);
