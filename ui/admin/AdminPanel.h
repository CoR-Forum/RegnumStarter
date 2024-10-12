#pragma once

// AdminPanel.cpp
#include "../../libs/imgui/imgui.h"
#include "UsersTable.h" // Adjust this path if necessary
#include <random>
#include <string>

extern std::string currentStatus;

void ShowAdminPanel(bool* show_admin_window);
extern void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);