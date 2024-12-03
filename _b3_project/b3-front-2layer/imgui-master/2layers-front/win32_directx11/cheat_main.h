// Cheat.h

#pragma once
#include <unordered_map>

class Cheat {
public:
    uintptr_t moduleBase;
    HANDLE hProcess;
    bool cfg_mod_menu = true;
    bool cfg_infinite_ammo = true;
    bool cfg_no_reload = false;
    bool cfg_aimbot = true;
    bool cfg_fly_hack = false;
    bool cfg_god_mode = false;
    bool cfg_unlimited_armor = false;
    bool cfg_esp = true;
    int cfg_add_money = 10000;
    int cfg_add_golden_keys = 1;
    int cfg_add_keys = 1;
    int cfg_add_skillPoints = 1;
    int aimbot_max_distance = 500;
    float cfg_esp_color[4] = { 255.0, 0.0, 0.0, 255.0 };

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
        float feet_x, feet_y, feet_z;
        float health;
        Entity* next;

        Entity();
    };

    // Read and Write process memory
    template<typename T> T RPM(SIZE_T address);
    template<typename T> void WPM(SIZE_T address, T buffer);


    //Cfg functions
    void readConfigFile(const std::string& filename, std::unordered_map<std::string, std::string>& config);
    void writeConfigFile(const std::string& filename);
    void resetConfig();

    // Cheat functions
    Entity* ESP();
    Vec2D AimbotCalcAngles(float playerX, float playerY, float playerZ, float enemyX, float enemyY, float enemyZ);
    void Aimbot();
    void flyHack();
    //void unlimitedArmor();
    void godMode();
    void infAMmo();
    void addSkillPoints(int points);
    void addMoney(int money);
};
