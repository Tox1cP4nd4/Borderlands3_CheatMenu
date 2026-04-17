# Borderlands 3 — External Cheat & Memory Research

> **Educational purposes only.** This project was developed to understand how external cheats
> interact with game processes, and more importantly, **how anti-cheat systems can detect them.**

---

> ⚠️ **Note:** Memory offsets are outdated and no longer match the current version of
> Borderlands 3. All memory-dependent features (ESP, Aimbot, God Mode, etc.) will **not
> function** with the current game build. This repository exists purely as a
> code reference and research artifact.

## Overview

External cheat tool for Borderlands 3, built entirely from scratch in **C++** using **DirectX 11**
and **Dear ImGui** for the overlay layer. The cheat runs as a separate process — no DLL injection —
reading and writing game memory externally via Windows API (`ReadProcessMemory` / `WriteProcessMemory`).

The overlay is rendered as a **fully transparent, click-through, always-on-top window**
(`WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE`) using DirectX 11's swap chain,
making it invisible to screenshot tools and basic detection methods.

---

## Project Structure & Build

> **Note:** The project path is deeply nested due to the ImGui example structure it was built on top of.
> The actual entry point is `cheat_main.cpp`, not `main.cpp`.

```
Borderlands3_CheatMenu/
└── _b3_project/
    └── b3-front-2layer/
        └── imgui-master/
            └── 2layers-front/
                └── win32_directx11/
                    ├── cheat_main.cpp   ← ENTRY POINT (start here)
                    ├── cheat_main.h
                    ├── cheat_init.cpp
                    └── cheat_init.h
```

## Features Implemented

### Local Player
| Feature | Description |
|---|---|
| **God Mode** | Continuously writes max health to player struct |
| **Unlimited Armor** | Locks armor value via memory patching |
| **Infinite Ammo** | Prevents ammo counter from decrementing |
| **No Reload** | Bypasses reload state machine |

### ESP (Extra Sensory Perception)
| Feature | Description |
|---|---|
| **Enemy Bounding Box** | 2D rectangle rendered over each enemy via `ImDrawList::AddRect` |
| **Health Bar** | Dynamic health bar scaled by distance — color shifts green → orange → red based on HP% |
| **Health % Label** | Rendered per-entity with color coding |
| **Distance Label** | World-space Z coordinate converted to in-game distance units |
| **3D ESP (WIP)** | Partial implementation of 3D bounding box projection (commented out) |
| **Configurable Color** | RGBA color picker via ImGui `ColorEdit4` |

### Aimbot
| Feature | Description |
|---|---|
| **Aimbot** | Locks aim toward nearest enemy entity |
| **Max Distance Slider** | Configurable range (0–500 units) |
| **Hotkey** | Activated via `VK_LMENU` (Left Alt) using `GetAsyncKeyState` |

### Misc
| Feature | Description |
|---|---|
| **Fly Hack** | Overrides player movement state — SPACE = up, SHIFT = down |
| **Add Money** | Writes arbitrary value to in-game currency pointer |
| **Add Skill Points** | Directly modifies skill point counter in memory |
| **Add Keys** | Normal and Golden Key manipulation |

### Config System
- Save/Load cheat configuration to `.CFG` files via Windows `OPENFILENAME` dialog
- Serializes all feature flags (bool, int, float) to key-value format
- Reset to defaults functionality

---

## Architecture

```
b3_project/
├── cheat_main.cpp       # Entry point: DirectX11 window, ImGui loop, feature dispatch
├── cheat_main.h         # Cheat class definition, feature flags, entity struct
├── cheat_init.cpp       # Process attachment: finds BL3 PID, gets module base address
├── cheat_init.h         # CheatHandler struct
└── imgui-master/        # Dear ImGui (DirectX11 + Win32 backend)
```

### How it works

```
1. Process Discovery
   └── OpenProcess(BL3.exe) → get hProcess + moduleBase

2. Overlay Creation
   └── CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST) → transparent DirectX11 window

3. Memory Reading (per frame)
   └── ReadProcessMemory(hProcess, moduleBase + offset_chain) → entity list

4. Render Loop
   └── ImGui::NewFrame()
       ├── ESP: iterate entity list → draw boxes via ImDrawList
       ├── Aimbot: find nearest entity → WriteProcessMemory(aim angles)
       └── Features: WriteProcessMemory(health/ammo/position pointers)
```

---

## Anti-Cheat Relevance

This project provided direct insight into the techniques that kernel-level anti-cheat systems
(Easy Anti-Cheat, BattlEye, Vanguard) are designed to detect:

| Attack Technique | Detection Vector |
|---|---|
| `OpenProcess` on game PID | Monitored by kernel driver — suspicious handle access |
| `ReadProcessMemory` in a loop | Pattern detected by memory access frequency analysis |
| External overlay (`WS_EX_TOPMOST`) | Window enumeration + layered window detection |
| Scanning for entity base address | Signature scanning patterns, pointer chain walking |
| Writing to health/ammo offsets | Integrity checks on protected memory regions |
| `GetAsyncKeyState` polling | Input hook detection from kernel mode |

Understanding the attacker's methodology — offset discovery, pointer chain traversal,
overlay rendering, config persistence — is fundamental to building effective detection systems.

---

## Technical Stack

- **Language:** C++ (C++17)
- **Graphics API:** DirectX 11 (`d3d11.h`, `DXGI`)
- **UI Framework:** [Dear ImGui](https://github.com/ocornut/imgui) (Win32 + DX11 backend)
- **Memory access:** Windows API (`ReadProcessMemory`, `WriteProcessMemory`, `OpenProcess`)
- **Window:** `WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE` — transparent overlay
- **Reverse Engineering tools used:** Cheat Engine, x64dbg, IDA Free

---

## Disclaimer

This project is for **educational and research purposes only**.
It was developed to understand game memory structures and external process interaction,
with the explicit goal of building better intuition for **anti-cheat engineering**.
Using cheats in online games violates Terms of Service. The author does not condone cheating.
