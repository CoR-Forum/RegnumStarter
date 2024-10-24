#pragma once

#ifndef ADMIN_PANEL_H
#define ADMIN_PANEL_H

#include "../../libs/imgui/imgui.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <ctime>
#include "../../libs/json.hpp"
#include "tables/AdminPanelTables.cpp"

using json = nlohmann::json;

static char statusInput[11] = ""; // 10 characters + null terminator

extern std::string sylentx_status;
extern std::string GetAllUsersRawJson;
extern std::string GetAllLicensesRawJson;

extern std::string generated_license_key; // Store the generated key returned by the API

extern bool setting_log_debug;

extern void ModifyGlobalSettings(const std::string& settingName, const std::string& settingValue);

void ShowAdminPanel(bool* show_admin_window);

void DisplayUsersTable();
void DisplayLicensesTable();

extern void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);

extern void ToggleUserBan(int userId);
extern void ToggleUserAdmin(int userId);
extern void ToggleUserActivation(int userId);

extern void ExpireLicense(int licenseId);

extern void SaveSettings();

#endif