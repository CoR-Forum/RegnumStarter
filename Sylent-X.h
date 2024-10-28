#ifndef SYLENT_X_H
#define SYLENT_X_H

#include <windows.h>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include "includes/Utils.h"
#include "includes/Updater/Updater.cpp"
#include "includes/Logger/Logger.cpp"
#include "includes/API/ApiHandler.cpp"
#include "includes/API/AdminApiHandler.cpp"
#include "Style.cpp"
#include "libs/DirectX/DirectXInit.cpp"
#include "libs/imgui/imgui_impl_dx9.cpp"
#include "libs/imgui/imgui_impl_win32.cpp"
#include "libs/ImageLoader/ImageLoader.cpp"
#include "libs/ImageLoader/FontAwesomeIcons.h"
#include "ui/helper/Markers/HelpMarker.cpp"
#include "ui/helper/Markers/LicenseMarker.cpp"
#include <filesystem>
#include "includes/process/process.cpp"
#include "includes/chrono/chrono.cpp"
#include "includes/streamproof/streamproof.cpp"
#include "ui/admin/AdminPanel.cpp"
#include "ui/login/pwreset/ForgotPasswordWindow.cpp"
#include "ui/login/pwreset/PasswordResetWindow.cpp"
#include "ui/helper/Rainbow/UpdateRainbowColor.cpp"
#include "ui/login/LoginWindow.cpp"
#include "ui/login/register/RegisterWindow.cpp"
#include "ui/RegnumStarter/RegnumStarter.cpp"
#include "ui/Feedback/FeedbackWindow.cpp"
#include "ui/License/LicenseWindow.cpp"
#include "ui/Movement/MovementWindow.cpp"
#include "ui/Credits/CreditsWindow.cpp"
#include "ui/Player/PlayerWindow.cpp"
#include "ui/View/ViewWindow.cpp"
#include "ui/Chat/ChatWindow.cpp"
#include "ui/WindowStates.h"
#include "includes/Bosses/BossSpawns.cpp"
#include "ui/Bosses/BossRespawnWindow.cpp"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "dwmapi.lib")

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern bool show_chat_window;
extern ImVec4 textColor;

extern bool g_ShowUI;
extern std::string statusMessage;
extern bool loginSuccess;

extern int userDefinedHotkey;
extern bool waitingForHotkey;

extern std::vector<Pointer> pointers;

extern const std::string regnumLoginUser;
extern const std::string regnumLoginPassword;

extern bool g_DeviceLost;
extern UINT g_ResizeWidth, g_ResizeHeight;
extern bool show_license_window;
extern bool spaceKeyPressed;
extern bool ctrlKeyPressed;
extern bool fovToggled;

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
std::wstring GetProcessPath(DWORD pid);

// Define MemoryAddress struct
struct MemoryAddress {
    std::string name;
    uintptr_t address;
    std::vector<unsigned long> offsets;
};

class Memory {
public:
    uintptr_t GetBaseAddress(const MemoryAddress& memAddr);
    bool WriteFloat(uintptr_t address, float value);
};

#endif // SYLENT_X_H