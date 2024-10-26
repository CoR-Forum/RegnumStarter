#pragma once

#include "../Utils.h"
#include "../InternetUtils/InternetUtils.h"
#include <stdexcept>
#include <sstream>
#include <regex>
#include "../md5/md5.h"

extern HWND hwnd;

// global variables for license key generation
std::string generated_license_key;

std::string GetAllLicensesRawJson;
std::string GetAllUsersRawJson;

void ModifyGlobalSettings(const std::string& settingName, const std::string& settingValue);
void GetAllLicenses();

void DisplayUsersTable();
void ToggleUserBan(int userId);
void ToggleUserAdmin(int userId);
void ToggleUserActivation(int userId);
void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);
