/*
* Cheat for Borderlands3 by h4wk0x01
*/

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include <unordered_map>
//#include <iostream>
#include <fstream>
#include <sstream>
#include "cheat_main.h"
#include "offsets.h"

#define UCONST_Pi 3.1415926

void Cheat::readConfigFile(const std::string& filename, std::unordered_map<std::string, std::string>& config) {
    std::ifstream file(filename);  // Open the configuration file

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignore empty lines or lines starting with #
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string key;
        std::string value;

        // Try to find the '=' separator
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            config[key] = value;
        }
    }

    file.close();
}


void Cheat::writeConfigFile(const std::string& filename) {
    std::ofstream outFile(filename);  // Open the file for writing

    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    std::unordered_map<std::string, std::string> config;
    config["cfg_infinite_ammo"] = cfg_infinite_ammo;
    config["cfg_no_reload"] = cfg_no_reload;
    config["cfg_aimbot"] = cfg_aimbot;
    config["cfg_fly_hack"] = cfg_fly_hack;
    config["cfg_god_mode"] = cfg_god_mode;
    config["cfg_unlimited_armor"] = cfg_unlimited_armor;
    config["cfg_esp"] = cfg_esp;
    config["cfg_add_money"] = cfg_add_money;
    config["cfg_add_golden_keys"] = cfg_add_golden_keys;
    config["cfg_add_keys"] = cfg_add_keys;
    config["cfg_add_skillPoints"] = cfg_add_skillPoints;
    config["aimbot_max_distance"] = aimbot_max_distance;
    config["cfg_esp_color_R"] = cfg_esp_color[0];
    config["cfg_esp_color_G"] = cfg_esp_color[1];
    config["cfg_esp_color_B"] = cfg_esp_color[2];
    config["cfg_esp_color_A"] = cfg_esp_color[3];

    // Write each key-value pair to the file in the format "key=value"
    for (const auto& pair : config) {
        outFile << pair.first << "=" << pair.second << std::endl;
    }

    outFile.close();  // Close the file after writing
}

void Cheat::resetConfig() {
    cfg_mod_menu = true;
    cfg_infinite_ammo = true;
    cfg_no_reload = false;
    cfg_aimbot = true;
    cfg_fly_hack = false;
    cfg_god_mode = false;
    cfg_unlimited_armor = false;
    cfg_esp = true;
    cfg_add_money = 10000;
    cfg_add_golden_keys = 1;
    cfg_add_keys = 1;
    cfg_add_skillPoints = 1;
    aimbot_max_distance = 500;
    cfg_esp_color[0] = 255.0;
    cfg_esp_color[1] = 0;
    cfg_esp_color[2] = 0;
    cfg_esp_color[3] = 255.0;
}

Cheat::Cheat(uintptr_t mb, HANDLE hp) : moduleBase(mb), hProcess(hp) {}

Cheat::Vec2D::Vec2D() : x(0), y(0) {}
Cheat::Vec2D::Vec2D(float _x, float _y) : x(_x), y(_y) {}

Cheat::Entity::Entity() : x(0), y(0), z(0), feet_x(0), feet_y(0), feet_z(0), health(0), next(nullptr){}

struct FRotator {
    float Yaw, Pitch, Roll;
};

Cheat::Vec3D::Vec3D() : x(0), y(0), z(0) {}
Cheat::Vec3D::Vec3D(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

Cheat::Vec3D RotationToVector(FRotator R)
{
    Cheat::Vec3D Vec;
    float fYaw = R.Yaw * UCONST_Pi / 180.0f;
    float fPitch = R.Pitch * UCONST_Pi / 180.0f;
    float CosPitch = cos(fPitch);
    Vec.x = cos(fYaw) * CosPitch;
    Vec.y = sin(fYaw) * CosPitch;
    Vec.z = sin(fPitch);

    return Vec;
}

float vecModule(Cheat::Vec3D& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void Normalize(Cheat::Vec3D& v)
{
    float size = vecModule(v);

    if (!size)
        v.x = v.y = v.z = 1;
    else
    {
        v.x /= size;
        v.y /= size;
        v.z /= size;
    }
}

void GetAxes(FRotator R, Cheat::Vec3D& X, Cheat::Vec3D& Y, Cheat::Vec3D& Z)
{
    X = RotationToVector(R); // up
    Normalize(X);
    R.Yaw += 99.0f;
    FRotator R2 = R;
    R2.Pitch = 35.0f;
    Y = RotationToVector(R2); // side
    Normalize(Y);
    Y.z = 0.0f;
    R.Yaw -= 100.0f;
    R.Pitch += 86.8f;
    Z = RotationToVector(R); // deep
    Normalize(Z);
}

Cheat::Vec3D VectorSubtract(Cheat::Vec3D va, Cheat::Vec3D vb)
{
    Cheat::Vec3D out;

    out.x = va.x - vb.x;
    out.y = va.y - vb.y;
    out.z = va.z - vb.z;

    return out;
}

float inline Dot(const Cheat::Vec3D& V1, const Cheat::Vec3D& V2)
{
    return (V1.x * V2.x + V1.y * V2.y + V1.z * V2.z);
}

Cheat::Vec3D WorldToScreen(Cheat::Vec3D EnemyLocation, FRotator RotationMatrix, Cheat::Vec3D PlayerLocation, float FOV)
{
    Cheat::Vec3D Return;

    Cheat::Vec3D AxisX, AxisY, AxisZ, Delta, Transformed;
    GetAxes(RotationMatrix, AxisX, AxisY, AxisZ);

    // In UE: Z(up), X(deep), Y(side)
    Delta = VectorSubtract(PlayerLocation, EnemyLocation);
    Transformed.x = Dot(Delta, AxisY);
    Transformed.y = Dot(Delta, AxisZ);
    Transformed.z = Dot(Delta, AxisX);

    if (Transformed.z < 1.00f)
        Transformed.z = 1.00f;

    float ScreenCenterX = 1980.0f / 2.0f;
    float ScreenCenterY = 1080.0f / 2.0f;
        
    Return.x = ScreenCenterX + Transformed.x * (ScreenCenterX / tan(FOV * UCONST_Pi / 360.0f)) / (Transformed.z);
    Return.y = ScreenCenterY + -Transformed.y * (ScreenCenterX / tan(FOV * UCONST_Pi / 360.0f)) / (Transformed.z);
    Return.z = Transformed.z;

    return Return;
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

template<typename T> T Cheat::RPM(SIZE_T address) {
    T buffer;
    ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
    return buffer;
}

template<typename T> void Cheat::WPM(SIZE_T address, T buffer) {
    WriteProcessMemory(hProcess, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

Cheat::Vec2D Cheat::AimbotCalcAngles(float playerX, float playerY, float playerZ, float enemyX, float enemyY, float enemyZ) {

    playerZ = playerZ + 10;
    float deltaX = enemyX - playerX;
    float deltaY = enemyY - playerY;
    float deltaZ = enemyZ - playerZ;

    // Calculando yaw (ângulo horizontal)
    float yaw = atan2(deltaY, deltaX);

    // Calculando pitch (ângulo vertical)
    float pitch = atan2(deltaZ, sqrt(deltaX * deltaX + deltaY * deltaY));

    // Convertendo para graus
    yaw = yaw * 180.0f / UCONST_Pi;
    pitch = pitch * 180.0f / UCONST_Pi;

    return Cheat::Vec2D(pitch, yaw);
}

void Cheat::Aimbot() {

    Vec3D playerCoords;
    Vec3D enemyCoords;

    // Pre-fetch common vars
    uintptr_t entityListPtrBase = moduleBase + dwEntityList;

    // Get local player coords
    playerCoords.x = RPM<float>(0x122BF0C8B10);
    playerCoords.y = RPM<float>(0x122BF0C8B14);
    playerCoords.z = RPM<float>(0x122BF0C8B18);

    float closestEnemyDistance = 999999;
    // Get closest enemy
    for (int i = 1; i < 30; i++) { // Change for ammount of enemies
        unsigned int currentEntity = 0x10 * i;

        // Get Enemy Coords(X,Y,Z)
        float enemyTmpX = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x220 }));
        float enemyTmpY = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x224 }));
        float enemyTmpZ = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x228 }));

        Vec3D delta = VectorSubtract(playerCoords, Vec3D(enemyTmpX, enemyTmpY, enemyTmpZ));
        float distance = vecModule(delta)/91;

        // Get Enemy health
        float enemyHealth = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0xA0 }));

        /*std::cout << "\n\nEnemy X: " << enemyTmpX << ", y: " << enemyTmpY << ", Z: " << enemyTmpZ << std::endl;
        std::cout << "Player X: " << playerCoords.x << ", y: " << playerCoords.y << ", Z: " << playerCoords.z << std::endl;
        std::cout << "Closest enemy distance: " << closestEnemyDistance << std::endl;
        std::cout << "Distance: " << distance << std::endl;*/
        if (distance < closestEnemyDistance && ((int)enemyHealth > 0 && (int)enemyHealth < 999)) { // If cloest Entity, and alive
            enemyCoords.x = enemyTmpX;
            enemyCoords.y = enemyTmpY;
            enemyCoords.z = enemyTmpZ;
            closestEnemyDistance = distance;
        }
    }

    std::cout << "closestEnemyDistance: " << closestEnemyDistance << std::endl;
    std::cout << "aimbot_max_distance: " << aimbot_max_distance << std::endl;
    if (closestEnemyDistance <= aimbot_max_distance) {
        Vec2D angles = AimbotCalcAngles(playerCoords.x, playerCoords.y, playerCoords.z+20, enemyCoords.x, enemyCoords.y, enemyCoords.z);
        uintptr_t PitchPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, PitchOffsets);
        uintptr_t YawPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, YawOffsets);
        WPM<float>(PitchPtr, angles.x); // Pitch
        WPM<float>(YawPtr, angles.y); // Yaw
    }
}

Cheat::Entity* Cheat::ESP() {

    // Entity List
    Entity* CurrentEntity = new Entity();
    Entity* FirstEntity = CurrentEntity;

    // Pre-fetch common vars
    uintptr_t entityListPtrBase = moduleBase + dwEntityList;

    // Pre-fetch common pointers
    uintptr_t PitchPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, PitchOffsets);
    uintptr_t YawPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, YawOffsets);
    uintptr_t FovPtr = FindDMAAddy(hProcess, moduleBase + dwFov, FovOffsets);

    for (int i = 1; i < 30; i++) { // Change for ammount of enemies
        unsigned int currentEntity = 0x10 * i;

        float pitch = RPM<float>(PitchPtr);
        float yaw = RPM<float>(YawPtr);
        float fov = RPM<float>(FovPtr);

        Vec3D PlayerLocation;
        PlayerLocation.x = RPM<float>(0x122BF0C8B10);
        PlayerLocation.y = RPM<float>(0x122BF0C8B14);
        PlayerLocation.z = RPM<float>(0x122BF0C8B18);

        // (Enemy Location)
        Vec3D EnemyLocation;
        EnemyLocation.x = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x220 }));
        EnemyLocation.y = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x224 }));
        EnemyLocation.z = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x228 }));

        /*std::cout << "Pitch: " << pitch << "°" << std::endl;
        std::cout << "Yaw:   " << yaw << "°" << std::endl;
        std::cout << "FOV:   " << fov << "°" << std::endl;
        std::cout << "Local Player X: " << PlayerLocation.x << std::endl;
        std::cout << "Local Player Y:   " << PlayerLocation.y << std::endl;
        std::cout << "Local Player Z:   " << PlayerLocation.z << std::endl;
        std::cout << "Enemy Player X: " << EnemyLocation.x << std::endl;
        std::cout << "Enemy Player Y:   " << EnemyLocation.y << std::endl;
        std::cout << "Enemy Player Z:   " << EnemyLocation.z << std::endl;*/

        // (Pitch, Yaw, Roll)
        FRotator Rotation;
        Rotation.Pitch = pitch;
        Rotation.Yaw = yaw;
        Rotation.Roll = 0.0f;

        Cheat::Vec3D aux = WorldToScreen(PlayerLocation, Rotation, EnemyLocation, fov); // W2S(Cheat::Vec3D Location, Cheat::Vec3D RotationMatrix, Cheat::Vec3D EnemyLocation, float FOV)
        CurrentEntity->next = new Entity();
        CurrentEntity->x = aux.x;
        CurrentEntity->y = aux.y;

        Vec3D delta = VectorSubtract(PlayerLocation, EnemyLocation);
        float distance = vecModule(delta);

        CurrentEntity->z = distance;

        // Get Enemy health
        float enemyHealth = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0xA0 }));

        // Get Max health
        float enemyMaxHealth = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0x20 }));

        if (!(enemyHealth > 0.0f && enemyHealth < 9999999)) { enemyHealth = 0; }
        CurrentEntity->health = (enemyHealth/enemyMaxHealth) * 100;

        //std::cout << "Enemy -> Screen Coordinates: (X:" << CurrentEntity->x << ", Y: " << CurrentEntity->y << ")" << std::endl;

        CurrentEntity = CurrentEntity->next;
    }
    return FirstEntity;
}

void Cheat::flyHack() {
   // uintptr_t playerX = FindDMAAddy(hProcess, moduleBase + 0x0686CAA0, { 0x68, 0xB88, 0x0, 0x168, 0x224 });
    //uintptr_t playerZ = FindDMAAddy(hProcess, moduleBase + 0x0686CAA0, PlayerZOffsets);

    //float currentX = 0;
    float currentZ = 0;

    currentZ = 0;//RPM<float>(sPlayerZ);
   // currentX = RPM<float>(playerX);
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) { // GO UP
        currentZ = currentZ + 300.0;
        WPM<float>(0, currentZ);
    }
    //if (GetAsyncKeyState(0x57) & 0x8000) { // MOVE FORWARD
    //    currentX = currentX + 300.0;
    //    WPM<float>(playerX, currentX);
    // }
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { // GO DOWN
        currentZ = currentZ - 50.0;
        WPM<float>(0, currentZ);
    }
}

//void Cheat::unlimitedArmor() {
//    uintptr_t playerArmorPtr = FindDMAAddy(hProcess, moduleBase + dwPlayerArmor, PlayerArmorOffsets);
//
//    float currentArmor = RPM<float>(playerArmorPtr);
//    if (currentArmor != 100) {
//        WPM<float>(playerArmorPtr, 100);
//    }
//}

void Cheat::infAMmo() {
    uintptr_t ammoPtr = FindDMAAddy(hProcess, moduleBase + dwInfAmmo, InfAmmoOffsets);
    WPM<float>(ammoPtr, 100);
}

void Cheat::godMode() {
    uintptr_t playerHealthPtr = FindDMAAddy(hProcess, moduleBase + dwEntityList, PlayerHealthOffsets);

    float currentHealth = RPM<float>(playerHealthPtr);
    if (currentHealth != 150) {
        WPM<float>(playerHealthPtr, 150);
    }
}

void Cheat::addSkillPoints(int points) {
    uintptr_t skillPointsAddr = FindDMAAddy(hProcess, moduleBase + dwSkillPoints, SkillPointsOffsets);
    int currentSkillPoints = RPM<int>(skillPointsAddr);
    WPM<int>(skillPointsAddr, currentSkillPoints + points);
}

void Cheat::addMoney(int money) {
    uintptr_t moneyAddr = FindDMAAddy(hProcess, moduleBase + dwMoney, MoneyOffsets);
    int currentMoney = RPM<int>(moneyAddr);
    WPM<int>(moneyAddr, currentMoney + money);
}
