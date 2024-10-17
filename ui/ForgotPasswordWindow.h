#pragma once

#ifndef FORGOT_PASSWORD_WINDOW_H
#define FORGOT_PASSWORD_WINDOW_H

#include <string>

void ShowForgotPasswordWindow(bool& show_forgot_password_window, bool& show_password_reset_window, bool& show_login_window);
extern void SaveSettings();
extern bool ResetPasswordRequest(const std::string& email);

#endif // FORGOT_PASSWORD_WINDOW_H