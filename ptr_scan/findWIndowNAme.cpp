#include <Windows.h>
#include <iostream>
#include <fstream>  // To save output to a file

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    // Get the window title
    char windowTitle[256];
    DWORD processID = 0;

    // Get the process ID of the window
    GetWindowThreadProcessId(hwnd, &processID);

    if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle))) {
        // Only print windows that have a non-empty title
        if (strlen(windowTitle) > 0) {
            // Open the output file (append mode)
            std::ofstream outFile("window_titles.txt", std::ios::app);

            if (outFile.is_open()) {
                // Print window title and PID to the file
                outFile << "Window Title: " << windowTitle << ", PID: " << processID << std::endl;
                outFile.close();
            }
        }
    }
    return TRUE; // Continue enumerating
}

int main() {
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
