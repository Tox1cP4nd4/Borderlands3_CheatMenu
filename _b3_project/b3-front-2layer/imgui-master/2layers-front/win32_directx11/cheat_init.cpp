#include <iostream>
#include <Windows.h>
#include <fstream>  // To save output to a file
#include <regex>  // For regular expression matching
#include <TlHelp32.h>

LPCSTR WINDOW_NAME = "Borderlands® 3";
wchar_t* MODULE_NAME = L"Borderlands3.exe";

DWORD ProcessID = 0;
uintptr_t moduleBase;
HWND hwnd;
HANDLE hProcess;

//REMEMBER TO ENABLE MULTI-BYTE CHARACTER SET ON VS PROJECT
uintptr_t GetModuleBaseAddress(const wchar_t* modName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, ProcessID);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!wcscmp(modEntry.szModule, modName)) {
                    CloseHandle(hSnap);
                    return (uintptr_t)modEntry.modBaseAddr;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd_local, LPARAM lParam) {
    // Get the window title
    char windowTitle[256];
    DWORD processID = 0;

    // Get the process ID of the window
    GetWindowThreadProcessId(hwnd_local, &processID);

    if (GetWindowTextA(hwnd_local, windowTitle, sizeof(windowTitle))) {
        // Only print windows that have a non-empty title
        if (strlen(windowTitle) > 0) {
            if (strlen(windowTitle) > 0) {
                // Open the output file (append mode)
                std::ofstream outFile("window_titles.txt", std::ios::app);

                if (outFile.is_open()) {
                    // Print window title and PID to the file
                    outFile << "Window Title: " << windowTitle << ", PID: " << processID << std::endl;
                    outFile.close();
                }
            }
            // Regex pattern to match "Borderlands" (case-insensitive)
            std::regex pattern(".*Borderlands® 3.*", std::regex_constants::icase);

            // Apply the regex to the window title
            if (std::regex_match(windowTitle, pattern)) {
                // Set the global process ID if a match is found
                ProcessID = processID;

                // Optionally print the results
                std::cout << "Found Borderlands window!" << std::endl;
                hwnd = hwnd_local;
                std::cout << "Window Title: " << windowTitle << ", PID: " << processID << ", HWND: " << hwnd << std::endl;

                // Save this to a file
                std::ofstream outFile("window_titles.txt", std::ios::app);
                if (outFile.is_open()) {
                    outFile << "Found Borderlands window!\n";
                    outFile << "Window Title: " << windowTitle << ", PID: " << processID << std::endl;
                    outFile.close();
                }

                // Stop enumerating after finding the first match
                return FALSE;
            }
        }
    }
    return TRUE; // Continue enumerating if no match is found
}

int findProcId() {
    // Open the file in write mode to start fresh
    std::ofstream outFile("window_titles.txt", std::ios::trunc);
    if (outFile.is_open()) {
        outFile << "List of Windows with Titles and PIDs\n";
        outFile << "--------------------------------------\n";
        outFile.close();
    }

    // Enumerate all open windows and print their titles and PIDs to the file
    EnumWindows(EnumWindowsProc, 0);

    std::cout << "Window titles and PIDs have been saved to 'window_titles.txt'.\n";
    return 0;
}

class CheatHandler {
public:
    DWORD ProcessID;
    uintptr_t moduleBase;
    HWND hwnd;
    HANDLE hProcess;

};

CheatHandler initializeCheat() {
    std::cout << "Initializing cheat...\n" << std::endl;

    CheatHandler handler;
    /*
    1) "handle to a window." It's a variable that stores a unique identifier for a window in a Windows operating system.
    The FindWindowA function is used to retrieve the handle of a window based on its window name.
    */
    //hwnd = FindWindowA(NULL, "Borderlands® 3");

    /*
    2) Get Proccess ID
    */
    findProcId();
    std::cout << "HWND: " << hwnd << std::endl;
    //GetWindowThreadProcessId(hwnd, &ProcessID);
    std::cout << "ProdID: " << ProcessID << std::endl;

    /*
    3) "moduleBase" -> Starting address in memory where a specific module (like a DLL or executable) is loaded
    */
    handler.moduleBase = GetModuleBaseAddress(MODULE_NAME);
    std::cout << "moduleBase: " << moduleBase << std::endl;

    /*
    4) hProcess is a handle to a process in the Windows operating system.
    hProcess serves as a reference to the specified process, allowing you to perform operations on it programmatically.
    */
    handler.hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, ProcessID);

    return handler;
}
