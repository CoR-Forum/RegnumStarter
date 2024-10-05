#include "Utils.h"

// Global variables for login window
HWND hLogin, hPassword, hLoginButton, hRegisterButton;
HWND hLogDisplay = nullptr; // Handle to the log display control

extern HHOOK hKeyboardHook;
extern std::atomic<bool> isWriting;
extern std::thread memoryThread;
extern bool isGravityKeyPressed;
extern bool optionGravity;
extern bool optionMoonjump;
extern bool optionZoom;
extern bool optionMoonwalk;
extern bool featureGravity;
extern bool featureMoonjump;
extern bool featureZoom;
extern bool featureMoonwalk;

// Global hook handle
HHOOK hKeyboardHook;

// Handle keyboard input
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN && p->vkCode == VK_SPACE) {
            if (!isGravityKeyPressed) {
                isGravityKeyPressed = true;
                if (optionGravity) {
                    isWriting = true;
                    memoryThread = std::thread(ContinuousMemoryWrite, "gravity");
                }
            }
        } else if (wParam == WM_KEYUP && p->vkCode == VK_SPACE) {
            if (isGravityKeyPressed) {
                isGravityKeyPressed = false;
                isWriting = false;
                if (memoryThread.joinable()) {
                    memoryThread.join();
                }
            }
        }

        if (wParam == WM_KEYDOWN && p->vkCode == VK_LCONTROL) {
            if (!isGravityKeyPressed) {
                isGravityKeyPressed = true;
                if (optionGravity) {
                    isWriting = true;
                    memoryThread = std::thread(ContinuousMemoryWrite, "gravitydown");
                }
            }
        } else if (wParam == WM_KEYUP && p->vkCode == VK_LCONTROL) {
            if (isGravityKeyPressed) {
                isGravityKeyPressed = false;
                isWriting = false;
                if (memoryThread.joinable()) {
                    memoryThread.join();
                }
            }        
        } 
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND chkoptionGravity, chkoptionMoonjump, chkoptionZoom, hLogoutButton,chkoptionMoonwalk;
    static HINSTANCE hInstance = GetModuleHandle(NULL);

    switch (msg) {
        case WM_CREATE:
            // Create checkboxes
            chkoptionGravity = CreateWindow("BUTTON", "Enable Gravity", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 20, 50, 150, 20, hwnd, (HMENU)1, NULL, NULL);
            chkoptionMoonjump = CreateWindow("BUTTON", "Enable Moonjump", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 20, 80, 150, 20, hwnd, (HMENU)2, NULL, NULL);
            chkoptionZoom = CreateWindow("BUTTON", "Enable Zoom", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 20, 110, 150, 20, hwnd, (HMENU)3, NULL, NULL);
            hLogDisplay = CreateWindow("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY, 20, 200, 760, 100, hwnd, NULL, NULL, NULL);
            chkoptionMoonwalk = CreateWindow("BUTTON", "Enable Moonwalk", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 20, 140, 150, 20, hwnd, (HMENU)4, NULL, NULL);
            // Disable checkboxes by default
            EnableWindow(chkoptionGravity, FALSE);
            EnableWindow(chkoptionMoonjump, FALSE);
            EnableWindow(chkoptionZoom, FALSE);
            EnableWindow(chkoptionMoonwalk, FALSE);

            // Create the Logout button
            hLogoutButton = CreateWindow("BUTTON", "Logout", WS_VISIBLE | WS_CHILD, 10, 10, 80, 25, hwnd, (HMENU)4, NULL, NULL);

            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Fill the background with the custom color
            HBRUSH hBrush = CreateSolidBrush(RGB(2, 2, 2)); // Custom background color (white)
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_SIZE:
            if (hLogDisplay) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                int logHeight = 100;
                SetWindowPos(hLogDisplay, NULL, 20, rect.bottom - logHeight - 20, rect.right - 40, logHeight, SWP_NOZORDER);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                optionGravity = !optionGravity;
                SendMessage(chkoptionGravity, BM_SETCHECK, optionGravity ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Gravity toggled");
            }
            if (LOWORD(wParam) == 2) {
                optionMoonjump = !optionMoonjump;
                SendMessage(chkoptionMoonjump, BM_SETCHECK, optionMoonjump ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Moonjump toggled");
                MemoryManipulation("moonjump");
            }
            if (LOWORD(wParam) == 3) {
                optionZoom = !optionZoom;
                SendMessage(chkoptionZoom, BM_SETCHECK, optionZoom ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Zoom toggled");
                MemoryManipulation("zoom");
            }
            if (LOWORD(wParam) == 4) {
                optionMoonwalk = !optionMoonwalk;
                SendMessage(chkoptionMoonwalk, BM_SETCHECK, optionMoonwalk ? BST_CHECKED : BST_UNCHECKED, 0);
                Log("Moonwalk toggled");
                MemoryManipulation("moonwalk");
            }
            if (LOWORD(wParam) == 4) {
                Log("Logout button clicked");
                Logout();
            }
            break;

        case WM_DESTROY:
            SaveSettings();  // Save settings on exit

            PostQuitMessage(0);
            break;

        case WM_START_SELF_UPDATE:
            SelfUpdate();
            SendMessage(hwnd, WM_ENABLE_CHECKBOXES, 0, 0);
            break;

        case WM_ENABLE_CHECKBOXES: // Custom message to enable checkboxes after login
            if (featureGravity == 1) {
                EnableWindow(chkoptionGravity, TRUE);
            } else {
                EnableWindow(chkoptionGravity, FALSE);
            }

            if (featureZoom == 1) {
                EnableWindow(chkoptionZoom, TRUE);
            } else {
                EnableWindow(chkoptionZoom, FALSE);
            }

            if (featureMoonjump == 1) {
                EnableWindow(chkoptionMoonjump, TRUE);
            } else {
                EnableWindow(chkoptionMoonjump, FALSE);
            }
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

bool RegisterMainWindowClass(HINSTANCE hInstance, const char* className) {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    return RegisterClassEx(&wc);
}

HWND CreateMainWindow(HINSTANCE hInstance, const char* className, const std::string& version, int nCmdShow) {
    std::string windowTitle = "Sylent-X " + version;
    HWND hwnd = CreateWindowEx(0, className, windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 756, 504, NULL, NULL, hInstance, NULL);

    if (hwnd != NULL) {
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);
    }

    return hwnd;
}

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