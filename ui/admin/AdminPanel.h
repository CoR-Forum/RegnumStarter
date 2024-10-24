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

// Global variables
extern std::string sylentx_status;
extern std::string GetAllUsersRawJson;
extern std::string GetAllLicensesRawJson;

extern std::string generated_license_key; // Store the generated key returned by the API

extern bool setting_log_debug;

extern void ModifyGlobalSettings(const std::string& settingName, const std::string& settingValue);

// Function to display the Admin Panel window
void ShowAdminPanel(bool* show_admin_window);

// Function to display the Users table in the Admin Panel
void DisplayUsersTable();

// Function to display the Licenses table in the Admin Panel
void DisplayLicensesTable();

extern void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);

extern void ToggleUserBan(int userId);
extern void ToggleUserAdmin(int userId);
extern void ToggleUserActivation(int userId);

extern void ExpireLicense(int licenseId);

#endif