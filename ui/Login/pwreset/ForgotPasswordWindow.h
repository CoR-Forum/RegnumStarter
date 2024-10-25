#pragma once

#ifndef FORGOT_PASSWORD_WINDOW_H
#define FORGOT_PASSWORD_WINDOW_H

#include <string>

static char forgotPasswordEmail[128] = "";

// Function to display the Forgot Password window
/**
 * @brief Displays the Forgot Password window and manages the visibility of related windows.
 * 
 * This function is responsible for showing the Forgot Password window and updating the visibility
 * states of the password reset and login windows based on user interactions.
 * 
 * @param show_forgot_password_window A reference to a boolean that determines whether the Forgot Password window is shown.
 * @param show_password_reset_window A reference to a boolean that determines whether the Password Reset window is shown.
 * @param show_login_window A reference to a boolean that determines whether the Login window is shown.
 */
void ShowForgotPasswordWindow(bool& show_forgot_password_window, bool& show_password_reset_window, bool& show_login_window);

// Function to save settings (externally defined)
extern void SaveSettings();

// Function to send a reset password request (externally defined)
// Returns true if the request was successful, false otherwise
extern bool ResetPasswordRequest(const std::string& email);

#endif // FORGOT_PASSWORD_WINDOW_H