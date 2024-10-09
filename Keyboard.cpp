#include "includes/Utils.h"

extern HHOOK hKeyboardHook;
extern std::atomic<bool> isWriting;
extern std::thread memoryThread;
extern bool isGravityKeyPressed;
extern bool optionGravity;
extern bool optionMoonjump;
extern bool optionZoom;
extern bool optionMoonwalk;
extern bool optionFov;
extern bool featureGravity;
extern bool featureZoom;
extern bool featureFov;
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
