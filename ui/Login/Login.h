#pragma once

#include "../../includes/API/ApiHandler.h"
#include "../../libs/imgui/imgui.h"
#include <thread>
#include <chrono>

void ShowLoginWindow(bool& show_login_window, std::string& statusMessage, bool& loginSuccess, bool& show_main_window, ImVec4 textColor);