// ASScheat.cpp : Cheat for AssaultCube v1.3.0.2 by h4wk0x01

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <string>
#include <iomanip> // for std::setprecision, std::fixed
#include <algorithm>  // For std::min

#define NUM_PLAYERS 0x18AC0C
#define X_COORD 0x4
#define Y_COORD 0x8
#define Z_COORD 0xC
#define LIFE_OFFSET 0xEC

LPCSTR WINDOW_NAME = "Borderlands® 3";
LPCSTR MODULE_NAME = "Borderlands3.exe";

//uintptr_t gameModuleName;
uintptr_t moduleBase;
DWORD procId;
HWND hwnd;
HANDLE hProcess;
uintptr_t MVPMatrixAddr = 0x0057DFD0;

// Define a 3D vector structure for position in the world (x, y, z)
struct Vec3 {
    float x, y, z;
};

// Define a 2D vector structure for screen coordinates (x, y)
struct Vec2D {
    float x, y;
};

//REMEMBER TO ENABLE MULTI-BYTE CHARACTER SET ON VS PROJECT
uintptr_t GetModuleBaseAddress(const char* modName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!strcmp(modEntry.szModule, modName)) {
                    CloseHandle(hSnap);
                    return (uintptr_t)modEntry.modBaseAddr;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
}

bool WorldToScreen(Vec3 worldPos, Vec2D& screenPos, float viewMatrix[16], int screenWidth, int screenHeight)
{

    float xClip = viewMatrix[0] * worldPos.x + viewMatrix[4] * worldPos.y + viewMatrix[8] * worldPos.z + viewMatrix[12];
    float yClip = viewMatrix[1] * worldPos.x + viewMatrix[5] * worldPos.y + viewMatrix[9] * worldPos.z + viewMatrix[13];
    float zClip = viewMatrix[2] * worldPos.x + viewMatrix[6] * worldPos.y + viewMatrix[10] * worldPos.z + viewMatrix[14];
    testNormalizedZ = zClip;
    float w = viewMatrix[3] * worldPos.x + viewMatrix[7] * worldPos.y + viewMatrix[11] * worldPos.z + viewMatrix[15];

    // Debug output to check the intermediate values
    /*std::cout << "Clip X: " << xClip << std::endl;
    std::cout << "Clip Y: " << yClip << std::endl;
    std::cout << "Clip Z: " << zClip << std::endl;
    std::cout << "W: " << w << std::endl;*/

    // If w is too small, the point is too far away or behind the camera, so we discard it
    if (w < 0.01f)
        return false;

    // Normalize the coordinates by dividing by W (perspective division)
    float normalizedX = xClip / w;
    float normalizedY = yClip / w;
    float normalizedZ = zClip / w;

    // Convert normalized coordinates into actual screen space
    // [-1, 1] -> [0, 1] for screen space
    float screenX = (screenWidth / 2 * normalizedX) + (normalizedX + screenWidth / 2);
    float screenY = -(screenHeight / 2 * normalizedY) + (normalizedY + screenHeight / 2);

    // Set the final screen position
    screenPos.x = screenX;
    screenPos.y = screenY;

    return true;
}

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
        addr += offsets[i];
    }
    return addr;
}

template<typename T> T RPM(SIZE_T address) {
    T buffer;
    ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
    return buffer;
}

template<typename T> void WPM(SIZE_T address, T buffer) {
    WriteProcessMemory(hProcess, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

// Function to draw a box at the specified screen coordinates
void DrawBox(HDC hdc, POINT topLeft, POINT bottomRight) {
    Rectangle(hdc, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}

void initialize() {
    std::cout << "Initializing cheat...\n" << std::endl;
    /*
    1) "handle to a window." It's a variable that stores a unique identifier for a window in a Windows operating system.
    The FindWindowA function is used to retrieve the handle of a window based on its window name.
    */
    hwnd = FindWindowA(NULL, WINDOW_NAME);
    std::cout << "HWND: " << hwnd << std::endl;

    /*
    2) Get Proccess ID
    */
    GetWindowThreadProcessId(hwnd, &procId);
    std::cout << "ProdID: " << procId << std::endl;

    /*
    3) "moduleBase" -> Starting address in memory where a specific module (like a DLL or executable) is loaded
    */
    moduleBase = GetModuleBaseAddress(MODULE_NAME);
    std::cout << "moduleBase: " << moduleBase << std::endl;

    /*
    4) hProcess is a handle to a process in the Windows operating system.
    hProcess serves as a reference to the specified process, allowing you to perform operations on it programmatically.
    */
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
}

int main()
{
    initialize();
}
