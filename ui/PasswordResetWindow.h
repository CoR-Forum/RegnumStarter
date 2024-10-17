#pragma once

#ifndef PASSWORD_RESET_WINDOW_H
#define PASSWORD_RESET_WINDOW_H

#include <string>

void ShowPasswordResetWindow(bool& show_password_reset_window, bool& show_login_window, bool& show_forgot_password_window);
extern void SaveSettings();
extern bool SetNewPassword(const std::string& token, const std::string& password);

#endif // PASSWORD_RESET_WINDOW_H