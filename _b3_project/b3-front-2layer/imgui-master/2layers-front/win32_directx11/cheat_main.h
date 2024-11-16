// Cheat.h

#pragma once

class Cheat {
public:
    uintptr_t moduleBase;
    HANDLE hProcess;

    // Constructor
    Cheat(uintptr_t mb, HANDLE hp);

    struct Vec2D {
        float x, y;

        Vec2D();
        Vec2D(float _x, float _y);
    };

    struct FVector {
        float x, y, z;
        FVector* next;
    };

    // Read and Write process memory
    template<typename T> T RPM(SIZE_T address);
    template<typename T> void WPM(SIZE_T address, T buffer);

    // Cheat functions
    FVector* ESP();
    void flyHack();
    void unlimitedArmor();
    void godMode();
};
