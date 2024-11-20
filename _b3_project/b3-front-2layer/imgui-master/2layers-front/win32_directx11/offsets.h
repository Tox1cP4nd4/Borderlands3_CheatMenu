#pragma once
#include <vector>



// Local Player -> Cam
std::vector<unsigned int> PitchOffsets = { 0x50, 0x520, 0x5B8, 0xF0, 0x760 };
std::vector<unsigned int> YawOffsets = { 0x50, 0x520, 0x5B8, 0xF0, 0x764 };
std::vector<unsigned int> FovOffsets = { 0x28, 0x130, 0x3F0, 0x118, 0x470 };

// Local Player -> Pos
std::vector<unsigned int> PlayerXOffsets = { 0x68, 0xB88, 0x0, 0x168, 0x220 };
std::vector<unsigned int> PlayerYOffsets = { 0x68, 0xB88, 0x0, 0x168, 0x224 };
std::vector<unsigned int> PlayerZOffsets = { 0x68, 0xB88, 0x0, 0x168, 0x228 };

std::vector<unsigned int> PlayerArmorOffsets = { 0x6D0, 0x190, 0x6D0, 0x180, 0x290 };
std::vector<unsigned int> PlayerHealthOffsets = { 0x6D0, 0x190, 0x6D0, 0x180, 0x198 };

// Enemy
std::vector<unsigned int> EnemyHealthOffsets = { 0xE70, 0x180, 0xA0 };

uintptr_t dwEntityList = 0x0686CAA0;
uintptr_t dwAngles = 0x06527058;
uintptr_t dwFov = 0x068536C8;
uintptr_t dwPlayerArmor = 0x069E9378;
uintptr_t dwPlayerHealth = 0x069E9378;

// StaticAddresses
uintptr_t sPlayerX = 0x7FF6969CAFE8;
uintptr_t sPlayerY = 0x7FF6969CAFEC;
uintptr_t sPlayerZ = 0x7FF6969CAFF0;
