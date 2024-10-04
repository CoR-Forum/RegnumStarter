#include "Utils.h"

// Global variables for login window
HWND hLogin, hPassword, hLoginButton, hRegisterButton;

// Login Window Procedure
LRESULT CALLBACK LoginWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            CreateWindow("STATIC", "Username or E-Mail:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 25, hwnd, NULL, NULL, NULL);
            hLogin = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 20, 150, 25, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 60, 80, 25, hwnd, NULL, NULL, NULL);
            hPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD, 100, 60, 150, 25, hwnd, NULL, NULL, NULL);

            hLoginButton = CreateWindow("BUTTON", "Login", WS_VISIBLE | WS_CHILD, 100, 100, 80, 25, hwnd, (HMENU)1, NULL, NULL);

            // button to register (open link in webbrowser)
            CreateWindow("BUTTON", "Register", WS_VISIBLE | WS_CHILD, 100, 140, 80, 25, hwnd, (HMENU)2, NULL, NULL);

            // button to open the website
            CreateWindow("BUTTON", "PwReset", WS_VISIBLE | WS_CHILD, 100, 180, 80, 25, hwnd, (HMENU)3, NULL, NULL);

            // Set the focus to the login edit control
            SetFocus(hLogin);

            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                char login[100], password[100];
                GetWindowText(hLogin, login, 100);
                GetWindowText(hPassword, password, 100);

                SaveLoginCredentials(login, password);

                DestroyWindow(hwnd);
            }

            if (LOWORD(wParam) == 2) {
                CreateRegistrationWindow((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
                DestroyWindow(hwnd); // Close the login window
            }

            if (LOWORD(wParam) == 3) {
                // open the password reset link in the default web browser
                ShellExecute(NULL, "open", "https://cort.cor-forum.de/#pwreset", NULL, NULL, SW_SHOWNORMAL);
            }
            break;

        case WM_DESTROY:
            // PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CreateLoginWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = LoginWindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = "LoginWindowClass";

    RegisterClass(&wc);

    hLoginWindow = CreateWindowEx(
        0,
        "LoginWindowClass",
        "Login",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 300,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hLoginWindow, SW_SHOW);
    SetForegroundWindow(hLoginWindow); // Bring the login window to the foreground
}

void OpenLoginWindow() {
    if (hLoginWindow && IsWindow(hLoginWindow)) {
        DestroyWindow(hLoginWindow);
        CreateLoginWindow(hInstance);
    } else {
        CreateLoginWindow(hInstance);
    }
}

LRESULT CALLBACK RegistrationWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hUsername, hEmail, hPassword, hRegisterButton;

    switch (msg) {
        case WM_CREATE:
            CreateWindow("STATIC", "Username:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 25, hwnd, NULL, NULL, NULL);
            hUsername = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 20, 150, 25, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "E-Mail:", WS_VISIBLE | WS_CHILD, 20, 60, 80, 25, hwnd, NULL, NULL, NULL);
            hEmail = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 60, 150, 25, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 100, 80, 25, hwnd, NULL, NULL, NULL);
            hPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD, 100, 100, 150, 25, hwnd, NULL, NULL, NULL);

            hRegisterButton = CreateWindow("BUTTON", "Register", WS_VISIBLE | WS_CHILD, 100, 140, 80, 25, hwnd, (HMENU)4, NULL, NULL);

            SetFocus(hUsername);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 4) {
                char username[100], email[100], password[100];
                GetWindowText(hUsername, username, 100);
                GetWindowText(hEmail, email, 100);
                GetWindowText(hPassword, password, 100);

                // Call the function to register the user using the REST API
                RegisterUser(username, email, password);
            }
            break;

        case WM_CLOSE_REGISTRATION_WINDOW:
            // Destroy the registration window and show a message box
            DestroyWindow(hwnd);
            MessageBox(NULL, "Registration successful. Please activate your account by clicking the link in the e-mail.", "Success", MB_ICONINFORMATION);
            // Open the login window
            OpenLoginWindow();
            break;

        case WM_DESTROY:
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CreateRegistrationWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = RegistrationWindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = "RegistrationWindowClass";

    RegisterClass(&wc);

    hRegistrationWindow = CreateWindowEx(
        0,
        "RegistrationWindowClass",
        "Register",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hRegistrationWindow, SW_SHOW);
}