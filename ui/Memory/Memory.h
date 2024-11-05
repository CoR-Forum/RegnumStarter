#ifndef MEMORY_H
#define MEMORY_H

#include "../../includes/Utils.h"
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <atomic>
#include <thread>

// Structure to hold pointer information
struct Pointer {
    std::string name;
    uintptr_t address;
    std::vector<uintptr_t> offsets;
};

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

// Function declarations
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
DWORD GetProcessIdByName(const std::wstring& processName);
std::wstring GetProcessPath(DWORD pid);
uintptr_t GetBaseAddress(const Pointer& pointer);
bool WriteFloat(uintptr_t address, float value);

#endif // MEMORY_H