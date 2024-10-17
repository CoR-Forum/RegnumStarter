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

using json = nlohmann::json;

// Global variables
extern std::string currentStatus; // Current status message
extern std::string GetAllUsersRawJson; // Raw JSON string of all users
extern std::string GetAllLicensesRawJson; // Raw JSON string of all licenses
extern std::string generatedLicenseKey; // Generated license key

extern bool setting_log_debug; // Debug logging setting

// Function to display the Admin Panel window
void ShowAdminPanel(bool* show_admin_window);

// Function to display the Users table in the Admin Panel
void DisplayUsersTable();

// Function to display the Licenses table in the Admin Panel
void DisplayLicensesTable();

// Function to generate a new license
extern void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);

// Functions to toggle user states
extern void ToggleUserBan(int userId);
extern void ToggleUserAdmin(int userId);
extern void ToggleUserActivation(int userId);

// Function to expire a license
extern void ExpireLicense(int licenseId);

#endif // ADMIN_PANEL_H