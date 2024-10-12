// admin/AdminPanel.h
#pragma once

extern std::string currentStatus;

void ShowAdminPanel(bool* show_admin_window);
extern void GenerateNewLicense(const std::string& licensedFeatures, const std::string& runtime);
extern std::string generatedLicenseKey;