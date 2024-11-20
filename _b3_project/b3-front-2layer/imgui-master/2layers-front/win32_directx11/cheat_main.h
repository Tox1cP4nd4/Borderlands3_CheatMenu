// Cheat.h

#pragma once

class Cheat {
public:
    uintptr_t moduleBase;
    HANDLE hProcess;

    //float closestEnemyDistance = 999999.0;

    // Constructor
    Cheat(uintptr_t mb, HANDLE hp);

    struct Vec2D {
        float x, y;

        Vec2D();
        Vec2D(float _x, float _y);
    };

    struct FVector {
        float x, y, z;

        FVector();
        FVector(float _x, float _y, float _z);
    };

    struct Entity {
        float x, y, z;
        float health;
        Entity* next;
    };

    // Read and Write process memory
    template<typename T> T RPM(SIZE_T address);
    template<typename T> void WPM(SIZE_T address, T buffer);

    // Cheat functions
    Entity* ESP();
    Vec2D AimbotCalcAngles(float playerX, float playerY, float playerZ, float enemyX, float enemyY, float enemyZ);
    void Aimbot();
    void flyHack();
    void unlimitedArmor();
    void godMode();
};
