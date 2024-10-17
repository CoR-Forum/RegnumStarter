#pragma once

// AdminPanel.cpp
#include "../../libs/imgui/imgui.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <ctime>
#include "../../libs/json.hpp"

using json = nlohmann::json;

extern std::string currentStatus;
extern std::string GetAllUsersRawJson;
extern std::string GetAllLicensesRawJson;
extern std::string generatedLicenseKey;

extern bool setting_log_debug;

void ShowAdminPanel(bool* show_admin_window);

void DisplayUsersTable();
void DisplayLicensesTable();

extern void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);

extern void ToggleUserBan(int userId);
extern void ToggleUserAdmin(int userId);
extern void ToggleUserActivation(int userId);

extern void ExpireLicense(int licenseId);