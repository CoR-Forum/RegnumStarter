#pragma once

#include "../../libs/imgui/imgui.h"

void ShowViewWindow(bool& show_view_window, bool& optionZoom, bool& optionFov, bool& featureFov, bool& waitingForHotkey, int& userDefinedHotkey);
std::string GetKeyName(int virtualKey);