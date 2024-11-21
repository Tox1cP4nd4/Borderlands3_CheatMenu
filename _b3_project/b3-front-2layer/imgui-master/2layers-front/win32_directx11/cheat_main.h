// Cheat.h

#pragma once

class Cheat {
public:
    uintptr_t moduleBase;
    HANDLE hProcess;
    bool cfg_mod_menu = true;
    bool cfg_infinite_ammo = false;
    bool cfg_no_reload = false;
    bool cfg_aimbot = false;
    bool cfg_fly_hack = false;
    bool cfg_god_mode = false;
    bool cfg_unlimited_armor = false;
    bool cfg_esp = true;
    float cfg_esp_color[4] = { 255.0, 0.0, 0.0, 255.0 };

    //float closestEnemyDistance = 999999.0;

    // Constructor
    Cheat(uintptr_t mb, HANDLE hp);

    struct Vec2D {
        float x, y;

        Vec2D();
        Vec2D(float _x, float _y);
    };

    struct Vec3D {
        float x, y, z;

        Vec3D();
        Vec3D(float _x, float _y, float _z);
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
    //Entity* loopEntityList();
    Entity* ESP();
    Vec2D AimbotCalcAngles(float playerX, float playerY, float playerZ, float enemyX, float enemyY, float enemyZ);
    void Aimbot();
    void flyHack();
    void unlimitedArmor();
    void godMode();
};
