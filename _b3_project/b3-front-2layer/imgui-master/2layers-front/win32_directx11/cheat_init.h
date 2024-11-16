#pragma once


class CheatHandler {
public:
    DWORD ProcessID;
    uintptr_t moduleBase;
    HWND hwnd;
    HANDLE hProcess;

    CheatHandler();
};

CheatHandler initializeCheat();
