// Cheat for Borderlands3 by h4wk0x01
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include "cheat_main.h"
#include "offsets.h"

#define UCONST_Pi 3.1415926

Cheat::Cheat(uintptr_t mb, HANDLE hp) : moduleBase(mb), hProcess(hp) {}

Cheat::Vec2D::Vec2D() : x(0), y(0) {}
Cheat::Vec2D::Vec2D(float _x, float _y) : x(_x), y(_y) {}

struct FRotator {
    float Yaw, Pitch, Roll;
};

Cheat::FVector::FVector() : x(0), y(0), z(0) {}
Cheat::FVector::FVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

Cheat::FVector RotationToVector(FRotator R)
{
    Cheat::FVector Vec;
    float fYaw = R.Yaw * UCONST_Pi / 180.0f;
    float fPitch = R.Pitch * UCONST_Pi / 180.0f;
    float CosPitch = cos(fPitch);
    Vec.x = cos(fYaw) * CosPitch;
    Vec.y = sin(fYaw) * CosPitch;
    Vec.z = sin(fPitch);

    return Vec;
}

float vecModule(Cheat::FVector M)
{
    return sqrt(M.x * M.x + M.y * M.y + M.z * M.z);
}

float Size(Cheat::FVector& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void Normalize(Cheat::FVector& v)
{
    float size = Size(v);

    if (!size)
        v.x = v.y = v.z = 1;
    else
    {
        v.x /= size;
        v.y /= size;
        v.z /= size;
    }
}

void GetAxes(FRotator R, Cheat::FVector& X, Cheat::FVector& Y, Cheat::FVector& Z)
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

Cheat::FVector VectorSubtract(Cheat::FVector va, Cheat::FVector vb)
{
    Cheat::FVector out;

    out.x = va.x - vb.x;
    out.y = va.y - vb.y;
    out.z = va.z - vb.z;

    return out;
}

float inline Dot(const Cheat::FVector& V1, const Cheat::FVector& V2)
{
    return (V1.x * V2.x + V1.y * V2.y + V1.z * V2.z);
}

struct FMinimalViewInfo {
    FRotator Rotation;
    Cheat::FVector Location;
    float FOV;

    FMinimalViewInfo(FRotator R, Cheat::FVector L, float F) : Rotation(R), Location(L), FOV(F) {};
};

// Working W2S !!
Cheat::FVector W2S(Cheat::FVector Location, FMinimalViewInfo cam_cache)
{
    Cheat::FVector Return;

    Cheat::FVector AxisX, AxisY, AxisZ, Delta, Transformed;
    GetAxes(cam_cache.Rotation, AxisX, AxisY, AxisZ);

    // In UE: Z(up), X(deep), Y(side)
    Delta = VectorSubtract(Location, cam_cache.Location);
    Transformed.x = Dot(Delta, AxisY);
    Transformed.y = Dot(Delta, AxisZ);
    Transformed.z = Dot(Delta, AxisX);
    std::cout << "Transformed.z: " << Transformed.z << "°" << std::endl;

    if (Transformed.z < 1.00f)
        Transformed.z = 1.00f;

    float ScreenCenterX = 1980.0f / 2.0f;
    float ScreenCenterY = 1080.0f / 2.0f;
        
    Return.x = ScreenCenterX + Transformed.x * (ScreenCenterX / tan(cam_cache.FOV * UCONST_Pi / 360.0f)) / (Transformed.z);
    Return.y = ScreenCenterY + -Transformed.y * (ScreenCenterX / tan(cam_cache.FOV * UCONST_Pi / 360.0f)) / (Transformed.z);
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

    FVector playerCoords;
    FVector enemyCoords;

    // Get local player coords
    playerCoords.x = RPM<float>(sPlayerX);
    playerCoords.y = RPM<float>(sPlayerY);
    playerCoords.z = RPM<float>(sPlayerZ);

    float closestEnemyDistance = 999999;
    // Get closest enemy
    for (int i = 1; i < 30; i++) { // Change for ammount of enemies
        unsigned int currentEntity = 0x10 * i;

        // Get Enemy Coords(X,Y,Z)
        uintptr_t enemyXptr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x220 });
        uintptr_t enemyYptr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x224 });
        uintptr_t enemyZptr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x228 });
        float enemyTmpX = RPM<float>(enemyXptr);
        float enemyTmpY = RPM<float>(enemyYptr);
        float enemyTmpZ = RPM<float>(enemyZptr);

        FVector delta = VectorSubtract(playerCoords, FVector(enemyTmpX, enemyTmpY, enemyTmpZ));
        float distance = vecModule(delta);

        // Get Enemy health
        uintptr_t enemyHealthPtr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0xA0 });
        float enemyHealth = RPM<float>(enemyHealthPtr);

        std::cout << "\n\nEnemy X: " << enemyTmpX << ", y: " << enemyTmpY << ", Z: " << enemyTmpZ << std::endl;
        std::cout << "Player X: " << playerCoords.x << ", y: " << playerCoords.y << ", Z: " << playerCoords.z << std::endl;
        std::cout << "Closest enemy distance: " << closestEnemyDistance << std::endl;
        std::cout << "Distance: " << distance << std::endl;
        if (distance < closestEnemyDistance && ((int)enemyHealth > 0 && (int)enemyHealth < 999)) { // If cloest Entity, and alive
            enemyCoords.x = enemyTmpX;
            enemyCoords.y = enemyTmpY;
            enemyCoords.z = enemyTmpZ;
            closestEnemyDistance = distance;
        }
    }

    Vec2D angles = AimbotCalcAngles(playerCoords.x, playerCoords.y, playerCoords.z, enemyCoords.x, enemyCoords.y, enemyCoords.z);

    WPM<float>(0x1C098EF121C, angles.x); // Pitch
    WPM<float>(0x1C098EF1220, angles.y); // Yaw
}

Cheat::Entity* Cheat::ESP() {

    //uintptr_t dwEntityList = RPM<DWORD>(moduleBase + 0x0686CAA0);
    Entity* screenPos = new Entity{ 0.0f, 0.0f, 0.0f, 0.0f, nullptr };
    Entity* head = screenPos;
    for (int i = 1; i < 30; i++) { // Change for ammount of enemies
        unsigned int currentEntity = 0x10 * i;

        uintptr_t PitchPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, PitchOffsets);
        float pitch = RPM<float>(0x1C098EF121C);

        uintptr_t YawPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, YawOffsets);
        float yaw = RPM<float>(0x1C098EF1220);

        uintptr_t FovPtr = FindDMAAddy(hProcess, moduleBase + dwFov, FovOffsets);
        float fov = RPM<float>(FovPtr);

        float LocalPlayerX = RPM<float>(sPlayerX);
        float LocalPlayerY = RPM<float>(sPlayerY);
        float LocalPlayerZ = RPM<float>(sPlayerZ);

        // (Enemy Location)
        FVector EnemyLocation;
        uintptr_t enemyX = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x220 });
        uintptr_t enemyY = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x224 });
        uintptr_t enemyZ = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x228 });
        EnemyLocation.x = RPM<float>(enemyX);
        EnemyLocation.y = RPM<float>(enemyY);
        EnemyLocation.z = RPM<float>(enemyZ);

        std::cout << "Pitch: " << pitch << "°" << std::endl;
        std::cout << "Yaw:   " << yaw << "°" << std::endl;
        std::cout << "FOV:   " << fov << "°" << std::endl;
        std::cout << "Local Player X: " << LocalPlayerX << std::endl;
        std::cout << "Local Player Y:   " << LocalPlayerY << std::endl;
        std::cout << "Local Player Z:   " << LocalPlayerZ << std::endl;
        std::cout << "Enemy Player X: " << EnemyLocation.x << std::endl;
        std::cout << "Enemy Player Y:   " << EnemyLocation.y << std::endl;
        std::cout << "Enemy Player Z:   " << EnemyLocation.z << std::endl;

        // (Pitch, Yaw, Roll)
        FRotator Rotation;
        Rotation.Pitch = pitch;
        Rotation.Yaw = yaw;
        Rotation.Roll = 0.0f;

        FMinimalViewInfo cam_cache(Rotation, EnemyLocation, fov);

        cam_cache.Location.x = LocalPlayerX;
        cam_cache.Location.y = LocalPlayerY;
        cam_cache.Location.z = LocalPlayerZ;

        /*FVector delta = VectorSubtract(cam_cache.Location, EnemyLocation);
        float vModule = vecModule(delta);*/

        Cheat::FVector aux = W2S(EnemyLocation, cam_cache);
        screenPos->next = new Entity{ aux.x, aux.y, aux.z, 0,nullptr };
        screenPos->x = aux.x;
        screenPos->y = aux.y;
        screenPos->z = aux.z;

        // Get Enemy health
        uintptr_t enemyHealthPtr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0xA0 });
        float enemyHealth = RPM<float>(enemyHealthPtr);
        if (!(enemyHealth > 0.0f && enemyHealth < 9999999)) { enemyHealth = 0; }
        screenPos->health = enemyHealth;

        std::cout << "Enemy -> Screen Coordinates: (X:" << screenPos->x << ", Y: " << screenPos->y << ")" << std::endl;

        /*std::cout << "3D Coordinates: (X: " << worldPos.x
            << ", Y: " << worldPos.y
            << ", Z: " << worldPos.z << ")" << std::endl;*/
        screenPos = screenPos->next;
    }
    return head;
    //}
}

void Cheat::flyHack() {
   // uintptr_t playerX = FindDMAAddy(hProcess, moduleBase + 0x0686CAA0, { 0x68, 0xB88, 0x0, 0x168, 0x224 });
    uintptr_t playerZ = FindDMAAddy(hProcess, moduleBase + 0x0686CAA0, PlayerZOffsets);

    //float currentX = 0;
    float currentZ = 0;

    currentZ = RPM<float>(playerZ);
   // currentX = RPM<float>(playerX);
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) { // GO UP
        currentZ = currentZ + 300.0;
        WPM<float>(playerZ, currentZ);
    }
    //if (GetAsyncKeyState(0x57) & 0x8000) { // MOVE FORWARD
    //    currentX = currentX + 300.0;
    //    WPM<float>(playerX, currentX);
    // }
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { // GO DOWN
        currentZ = currentZ - 50.0;
        WPM<float>(playerZ, currentZ);
    }
}

void Cheat::unlimitedArmor() {
    uintptr_t playerArmorPtr = FindDMAAddy(hProcess, moduleBase + dwPlayerArmor, PlayerArmorOffsets);

    float currentArmor = RPM<float>(playerArmorPtr);
    if (currentArmor != 100) {
        WPM<float>(playerArmorPtr, 100);
    }
}

void Cheat::godMode() {
    uintptr_t playerHealthPtr = FindDMAAddy(hProcess, moduleBase + dwPlayerHealth, PlayerHealthOffsets);

    float currentHealth = RPM<float>(playerHealthPtr);
    if (currentHealth != 150) {
        WPM<float>(playerHealthPtr, 150);
    }
}
