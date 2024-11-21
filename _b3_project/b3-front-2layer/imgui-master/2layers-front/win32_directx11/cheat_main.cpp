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
#include "cheat_main.h"
#include "offsets.h"

#define UCONST_Pi 3.1415926

Cheat::Cheat(uintptr_t mb, HANDLE hp) : moduleBase(mb), hProcess(hp) {}

Cheat::Vec2D::Vec2D() : x(0), y(0) {}
Cheat::Vec2D::Vec2D(float _x, float _y) : x(_x), y(_y) {}

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

// Working W2S !!
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
    //std::cout << "Transformed.z: " << Transformed.z << "°" << std::endl;

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
    playerCoords.x = RPM<float>(sPlayerX);
    playerCoords.y = RPM<float>(sPlayerY);
    playerCoords.z = RPM<float>(sPlayerZ);

    float closestEnemyDistance = 999999;
    // Get closest enemy
    for (int i = 1; i < 30; i++) { // Change for ammount of enemies
        unsigned int currentEntity = 0x10 * i;

        // Get Enemy Coords(X,Y,Z)
        float enemyTmpX = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x220 }));
        float enemyTmpY = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x224 }));
        float enemyTmpZ = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0x168, 0x228 }));

        Vec3D delta = VectorSubtract(playerCoords, Vec3D(enemyTmpX, enemyTmpY, enemyTmpZ));
        float distance = vecModule(delta);

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

    Vec2D angles = AimbotCalcAngles(playerCoords.x, playerCoords.y, playerCoords.z, enemyCoords.x, enemyCoords.y, enemyCoords.z);
    uintptr_t PitchPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, PitchOffsets);
    uintptr_t YawPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, YawOffsets);
    WPM<float>(PitchPtr, angles.x); // Pitch
    WPM<float>(YawPtr, angles.y); // Yaw
}

Cheat::Entity* Cheat::ESP() {

    // Entity List
    Entity* CurrentEntity = new Entity{ 0.0f, 0.0f, 0.0f, 0.0f, nullptr };
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
        PlayerLocation.x = RPM<float>(sPlayerX);
        PlayerLocation.y = RPM<float>(sPlayerY);
        PlayerLocation.z = RPM<float>(sPlayerZ);

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
        CurrentEntity->next = new Entity{ aux.x, aux.y, aux.z, 0, nullptr };
        CurrentEntity->x = aux.x;
        CurrentEntity->y = aux.y;
        CurrentEntity->z = aux.z;

        // Get Enemy health
        float enemyHealth = RPM<float>(FindDMAAddy(hProcess, entityListPtrBase, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0xA0 }));

        if (!(enemyHealth > 0.0f && enemyHealth < 9999999)) { enemyHealth = 0; }
        CurrentEntity->health = enemyHealth;

        //std::cout << "Enemy -> Screen Coordinates: (X:" << CurrentEntity->x << ", Y: " << CurrentEntity->y << ")" << std::endl;

        CurrentEntity = CurrentEntity->next;
    }
    return FirstEntity;
}

//Cheat::Entity* Cheat::loopEntityList() {
//
//    Vec3D playerCoords;
//    Vec3D enemyCoords;
//
//    // ESP variables
//    Entity* screenPos = new Entity{ 0.0f, 0.0f, 0.0f, 0.0f, nullptr };
//    Entity* head = screenPos;
//
//    float closestEnemyDistance = 999999;
//    // Get closest enemy
//    for (int i = 1; i < 3; i++) { // Change for ammount of enemies
//        unsigned int currentEntity = 0x10 * i;
//
//        // Get local player coords
//        playerCoords.x = RPM<float>(sPlayerX);
//        playerCoords.y = RPM<float>(sPlayerY);
//        playerCoords.z = RPM<float>(sPlayerZ);
//
//        // Get Enemy Coords(X,Y,Z)
//        uintptr_t enemyXptr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x220 });
//        uintptr_t enemyYptr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x224 });
//        uintptr_t enemyZptr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x228 });
//        float enemyTmpX = RPM<float>(enemyXptr);
//        float enemyTmpY = RPM<float>(enemyYptr);
//        float enemyTmpZ = RPM<float>(enemyZptr);
//
//        // Get Enemy health
//        uintptr_t enemyHealthPtr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0xA0 });
//        float enemyHealth = RPM<float>(enemyHealthPtr);
//
//        
//        if (esp) { // If ESP enabled
//
//            uintptr_t PitchPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, PitchOffsets);
//            uintptr_t YawPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, YawOffsets);
//            float pitch = RPM<float>(PitchPtr);
//            float yaw = RPM<float>(YawPtr);
//
//            uintptr_t FovPtr = FindDMAAddy(hProcess, moduleBase + dwFov, FovOffsets);
//            float fov = RPM<float>(FovPtr);
//
//
//            // (Enemy Location)
//            Vec3D EnemyLocation;
//            uintptr_t enemyX = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x220 });
//            uintptr_t enemyY = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x224 });
//            uintptr_t enemyZ = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0x168, 0x228 });
//            EnemyLocation.x = RPM<float>(enemyX);
//            EnemyLocation.y = RPM<float>(enemyY);
//            EnemyLocation.z = RPM<float>(enemyZ);
//
//            std::cout << "Pitch: " << pitch << "°" << std::endl;
//            std::cout << "Yaw:   " << yaw << "°" << std::endl;
//            std::cout << "FOV:   " << fov << "°" << std::endl;
//            std::cout << "Local Player X: " << playerCoords.x << std::endl;
//            std::cout << "Local Player Y:   " << playerCoords.y << std::endl;
//            std::cout << "Local Player Z:   " << playerCoords.z << std::endl;
//            std::cout << "Enemy Player X: " << EnemyLocation.x << std::endl;
//            std::cout << "Enemy Player Y:   " << EnemyLocation.y << std::endl;
//            std::cout << "Enemy Player Z:   " << EnemyLocation.z << std::endl;
//
//            // (Pitch, Yaw, Roll)
//            FRotator Rotation;
//            Rotation.Pitch = pitch;
//            Rotation.Yaw = yaw;
//            Rotation.Roll = 0.0f;
//
//            FMinimalViewInfo cam_cache(Rotation, EnemyLocation, fov);
//
//            cam_cache.Location.x = playerCoords.x;
//            cam_cache.Location.y = playerCoords.y;
//            cam_cache.Location.z = playerCoords.z; // Get distance for ESP distance adjustments
//
//            Cheat::Vec3D aux = W2S(EnemyLocation, cam_cache);
//            screenPos->next = new Entity{ aux.x, aux.y, aux.z, 0,nullptr };
//            screenPos->x = aux.x;
//            screenPos->y = aux.y;
//            screenPos->z = aux.z;
//
//            // Get Enemy health
//            uintptr_t enemyHealthPtr = FindDMAAddy(hProcess, moduleBase + dwEntityList, { 0x68, 0xB88, currentEntity, 0xE70, 0x180, 0xA0 });
//            float enemyHealth = RPM<float>(enemyHealthPtr);
//            if (!(enemyHealth > 0.0f && enemyHealth < 9999999)) { enemyHealth = 0; }
//            screenPos->health = enemyHealth;
//
//            std::cout << "Enemy -> Screen Coordinates: (X:" << screenPos->x << ", Y: " << screenPos->y << ")" << std::endl;
//
//            /*std::cout << "3D Coordinates: (X: " << worldPos.x
//                << ", Y: " << worldPos.y
//                << ", Z: " << worldPos.z << ")" << std::endl;*/
//            screenPos = screenPos->next;
//        }
//
//        if (aimbot) { // If Aimbot enabled
//            Vec3D delta = VectorSubtract(playerCoords, Vec3D(enemyTmpX, enemyTmpY, enemyTmpZ));
//            float distance = vecModule(delta);
//            std::cout << "\n\nEnemy X: " << enemyTmpX << ", y: " << enemyTmpY << ", Z: " << enemyTmpZ << std::endl;
//            std::cout << "Player X: " << playerCoords.x << ", y: " << playerCoords.y << ", Z: " << playerCoords.z << std::endl;
//            std::cout << "Closest enemy distance: " << closestEnemyDistance << std::endl;
//            std::cout << "Distance: " << distance << std::endl;
//            if (distance < closestEnemyDistance && ((int)enemyHealth > 0 && (int)enemyHealth < 999)) { // If cloest Entity, and alive
//                enemyCoords.x = enemyTmpX;
//                enemyCoords.y = enemyTmpY;
//                enemyCoords.z = enemyTmpZ;
//                closestEnemyDistance = distance;
//            }
//        }
//    }
//
//    if (aimbot) {
//        Vec2D angles = AimbotCalcAngles(playerCoords.x, playerCoords.y, playerCoords.z, enemyCoords.x, enemyCoords.y, enemyCoords.z);
//        uintptr_t PitchPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, PitchOffsets);
//        uintptr_t YawPtr = FindDMAAddy(hProcess, moduleBase + dwAngles, YawOffsets);
//        WPM<float>(PitchPtr, angles.x); // Pitch
//        WPM<float>(YawPtr, angles.y); // Yaw
//    }
//
//    return head;
//}

void Cheat::flyHack() {
   // uintptr_t playerX = FindDMAAddy(hProcess, moduleBase + 0x0686CAA0, { 0x68, 0xB88, 0x0, 0x168, 0x224 });
    //uintptr_t playerZ = FindDMAAddy(hProcess, moduleBase + 0x0686CAA0, PlayerZOffsets);

    //float currentX = 0;
    float currentZ = 0;

    currentZ = RPM<float>(sPlayerZ);
   // currentX = RPM<float>(playerX);
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) { // GO UP
        currentZ = currentZ + 300.0;
        WPM<float>(sPlayerZ, currentZ);
    }
    //if (GetAsyncKeyState(0x57) & 0x8000) { // MOVE FORWARD
    //    currentX = currentX + 300.0;
    //    WPM<float>(playerX, currentX);
    // }
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { // GO DOWN
        currentZ = currentZ - 50.0;
        WPM<float>(sPlayerZ, currentZ);
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
