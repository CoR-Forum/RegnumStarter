#pragma once

#ifndef FORGOT_PASSWORD_WINDOW_H
#define FORGOT_PASSWORD_WINDOW_H

#include <string>

// Function to display the Forgot Password window
void ShowForgotPasswordWindow(bool& show_forgot_password_window, bool& show_password_reset_window, bool& show_login_window);

// Function to save settings (externally defined)
extern void SaveSettings();

// Function to send a reset password request (externally defined)
// Returns true if the request was successful, false otherwise
extern bool ResetPasswordRequest(const std::string& email);

#endif // FORGOT_PASSWORD_WINDOW_H