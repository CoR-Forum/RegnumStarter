#pragma once

#include "includes/Utils.h"
#include <stdexcept>
#include <sstream>
#include <regex>
#include "ui/admin/AdminPanel.h"

extern HWND hwnd;

extern bool featureZoom;
extern bool featureGravity;
extern bool featureMoonjump;
extern bool featureMoonwalk;
extern bool featureFov;
extern bool featureSpeedhack;
extern bool featureFreecam;
extern bool featureFastfly;
extern bool featureFakelag;

std::string login;
std::string password;

std::string license_runtime_end;
std::string license_features;


std::string GetAllLicensesRawJson;
std::string GetAllUsersRawJson;
std::vector<Pointer> g_pointers;
std::vector<std::string> g_chatMessages;