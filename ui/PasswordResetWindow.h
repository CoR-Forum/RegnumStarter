#pragma once

#ifndef PASSWORD_RESET_WINDOW_H
#define PASSWORD_RESET_WINDOW_H

#include <string>

// Function to display the Password Reset window
void ShowPasswordResetWindow(bool& show_password_reset_window, bool& show_login_window, bool& show_forgot_password_window);

// Function to save settings (externally defined)
extern void SaveSettings();

// Function to set a new password using a token (externally defined)
// Returns true if the password was successfully set, false otherwise
extern bool SetNewPassword(const std::string& token, const std::string& password);

#endif // PASSWORD_RESET_WINDOW_H