#include "process.h"
#include <windows.h>
#include <tlhelp32.h>

bool IsProcessOpen(const std::string& processName) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    bool processFound = false;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    do {
        if (std::string(pe32.szExeFile) == processName) {
            processFound = true;
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return processFound;
}