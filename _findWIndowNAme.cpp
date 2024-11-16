#include <Windows.h>
#include <iostream>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    // Get the window title
    char windowTitle[256];
    if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle))) {
        // Only print windows that have a non-empty title
        if (strlen(windowTitle) > 0) {
            std::cout << windowTitle << std::endl;
        }
    }
    return TRUE; // Continue enumerating
}

int main() {
    // Enumerate all open windows and print their titles
    EnumWindows(EnumWindowsProc, 0);

    return 0;
}
