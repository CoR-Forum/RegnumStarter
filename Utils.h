#ifndef UTILS_H
#define UTILS_H

#include <string>

// Convert wstring to string
std::string WStringToString(const std::wstring& wstr) {
    std::string str(wstr.begin(), wstr.end());
    return str;
}

// Declare login and password globally
std::string login;
std::string password;

#endif // UTILS_H