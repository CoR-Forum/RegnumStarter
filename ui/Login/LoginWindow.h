#pragma once

#include "../../includes/API/ApiHandler.h"
#include "../../libs/imgui/imgui.h"
#include <thread>
#include <chrono>

extern const std::string regnumstarter_windowname;

void ShowLoginWindow(bool& show_login_window, std::string& statusMessage, bool& loginSuccess, bool& show_main_window, ImVec4 textColor);