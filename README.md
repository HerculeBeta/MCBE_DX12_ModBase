# MCBE DX12 Internal Client Template

A modular C++20 template for building internal DirectX 12 clients for Minecraft Bedrock Edition. Built on a clean event-driven architecture featuring an EnTT-based SDK, dynamic vtable hooking, ImGui DirectX 12 overlay rendering, Win32 input routing, and a JSON profile configuration system.

> [!NOTE]
> **Game Version & Compatibility**: Current signatures, vtable slots, and struct offsets are updated for Minecraft Bedrock **v26.40**.
> 
> **Template Base**: This repository is designed as a foundational template/base. It may or may not receive continuous SDK expansions or updates for future game versions. You are completely free to expand, modify the code, integrate it into your own client, or fully rebrand it under the terms of the MIT License.

---

## Features

- **DirectX 12 Overlay**: Native D3D12 render hook powered by Kiero with fence synchronization, buffer resize handling, and ImGui integration.
- **Event-Driven Pipeline**: Type-safe, decoupled `EventBus` supporting publish/subscribe patterns and event cancellation.
- **EnTT Component SDK**: Direct access to Minecraft Bedrock ECS (`EntityContext`, entity registries, and standard game components).
- **Dynamic VTable & Signature Hooks**: Memory scanning via `libhat` with compile-time member function vtable target extraction and MinHook detours.
- **Modular Feature Architecture**: Category-based module system supporting toggle/hold keybinds, runtime lifecycle callbacks, and typed serialized settings.
- **JSON Profile Manager**: Automatic profile serialization and deserialization for module states and configuration values.
- **Modern C++20 Core**: Structured logging with `std::source_location` and `std::format`, RAII memory management, and clean CMake build configuration.

---

## Project Structure

```text
MCBE_DX12_ModBase/
├── CMakeLists.txt              # CMake build definitions and compiler flags
├── README.md                   # Project documentation
│
├── src/
│   ├── client.h / .cpp         # Client singleton lifecycle (init, hooks, shutdown)
│   │
│   ├── config/                 # Configuration & profile persistence
│   │   ├── config_manager.h    # JSON profile save/load/delete manager
│   │   └── setting.h           # Typed settings (Float, Int, Bool, Enum)
│   │
│   ├── events/                 # EventBus & runtime event payloads
│   │   ├── event.h             # Base cancellable Event class
│   │   ├── event_bus.h         # Type-safe EventBus singleton
│   │   ├── actor_tick_event.h  # Dispatched on player/actor tick
│   │   ├── attack_event.h      # Dispatched on GameMode::attack
│   │   ├── client_instance_update_event.h
│   │   └── key_event.h         # Dispatched on Win32 key presses
│   │
│   ├── input/                  # Win32 input processing & keybind tracking
│   │   ├── input_manager.h     # Input routing & keybind listening state
│   │   └── key_names.h         # Virtual key code to display name mapping
│   │
│   ├── minecraft/              # Bedrock memory, hooks, and SDK
│   │   ├── bedrock_memory.h    # Signatures, vtable indices, and struct offsets
│   │   ├── hooks/              # MinHook installations & memory helpers
│   │   │   ├── hook_manager.h  # Master hook lifecycle orchestrator
│   │   │   ├── hook_memory.h   # libhat pattern scanner & vtable helpers
│   │   │   ├── actor_tick_hook.h
│   │   │   ├── attack_hook.h
│   │   │   └── client_instance_hook.h
│   │   └── sdk/                # Reverse-engineered game classes & EnTT ECS
│   │       ├── actor/          # Actor, LocalPlayer, GameMode
│   │       ├── client/         # ClientInstance, LevelRenderer, GuiData, GLMatrix
│   │       ├── components/     # StateVector, Rotation, AABB, etc.
│   │       └── entity/         # EntityContext, EntityId, EnTT integration
│   │
│   ├── modules/                # Base module system and feature modules
│   │   ├── module.h / .cpp     # Module base class
│   │   ├── module_manager.h    # Module registry and category manager
│   │   ├── combat/             # Combat modules (e.g. Killaura)
│   │   ├── movement/           # Movement modules (e.g. Jetpack, Fly)
│   │   ├── player/             # Player utility modules
│   │   └── visual/             # Render & HUD modules (e.g. Arraylist, ESP)
│   │
│   ├── platform/windows/       # DLL entry point (DllMain) and uninject thread
│   │
│   ├── render/dx12/            # DirectX 12 hook & ImGui backend management
│   │   └── dx12_hook.h / .cpp  # SwapChain Present/ResizeBuffers hooks & fence sync
│   │
│   ├── ui/                     # ImGui UI design, theme styling, and widgets
│   │   ├── menu.h / .cpp       # Main client GUI overlay
│   │   ├── ui_settings.h       # UI scale and rendering settings
│   │   └── widgets/            # Custom switches, sliders, search bar, badges
│   │
│   └── utils/                  # Utilities, logging, and math
│       ├── logger.h / .cpp     # Thread-safe format logger
│       └── math.h              # Vec2, Vec3, ViewAngles, AABB math types
│
└── external/
    ├── d3d12/                  # DirectX 12 helper headers
    ├── entt/                   # EnTT entity component system
    ├── imgui/                  # Dear ImGui library & backends
    ├── kiero/                  # D3D12 hooking library
    ├── libhat/                 # SIMD signature scanning library
    ├── minhook/                # x64 API hook library
    └── nlohmann/               # JSON for Modern C++
```

---

## Requirements

- **Operating System**: Windows 10 or Windows 11 (x64)
- **Compiler / IDE**: Visual Studio 2022 (MSVC v143+) with the *Desktop development with C++* workload
- **Build System**: CMake 3.20 or newer
- **C++ Standard**: C++20 (`/std:c++20`)

---

## Build & Usage

### 1. Build via Visual Studio
1. Open the project folder in Visual Studio 2022 (**File > Open > Folder**).
2. Set the configuration target to **x64-Release**.
3. Build the project (**Build > Build All** or `Ctrl+Shift+B`).
4. Output DLL will be generated in `build/Release/MCBE_DX12_ModBase.dll` or `out/build/x64-Release/MCBE_DX12_ModBase.dll`.

### 2. Build via CMake CLI
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### 3. Injection & Controls
Inject the compiled DLL into `Minecraft.Windows.exe` using any standard 64-bit injector.

| Hotkey | Action |
| :--- | :--- |
| `INSERT` | Toggle client ClickGUI menu |
| `END` | Cleanly unhook, release resources, and unload DLL |

### 4. Configuration & Profiles
Client profile data and log outputs are automatically stored in the `%APPDATA%` directory:
- **Configuration Profiles**: `%APPDATA%\MCBE_DX12_ModBase\configs\<profile>.json`
- **Client Logs**: `%APPDATA%\MCBE_DX12_ModBase\logs\latest.log`

---

## Developer Guide

### 1. Creating a New Module

Create a header and source file in the appropriate `src/modules/<category>/` folder:

```cpp
#pragma once
#include "modules/module.h"
#include "events/event_bus.h"
#include "events/actor_tick_event.h"
#include "minecraft/sdk/actor/local_player.h"

class Jetpack final : public Module {
public:
    Jetpack() : Module("Movement", "Jetpack", "Flies in the direction you look.") {
        m_speed = add_setting<config::FloatSetting>("Speed", "Flight velocity multiplier.", 1.5f, 0.1f, 10.0f);
    }

    void on_enable() override {
        m_tickListener = EventBus::instance().subscribe<ActorTickEvent>([this](ActorTickEvent& event) {
            if (!event.localPlayer) return;

            math::Vec3 direction = math::get_direction(event.localPlayer->getRotation());
            event.localPlayer->setVelocity(direction * m_speed->value());
        });
    }

    void on_disable() override {
        EventBus::instance().unsubscribe(m_tickListener);
    }

private:
    EventBus::ListenerId m_tickListener = 0;
    config::FloatSetting* m_speed = nullptr;
};
```

Register the module in `src/modules/module_manager.cpp`:

```cpp
m_modules.push_back(std::make_unique<Jetpack>());
```

CMake will automatically pick up any new `.cpp` and `.h` files under `src/`.

---

### 2. Working with the EventBus

#### Subscribing to Events
```cpp
EventBus::ListenerId id = EventBus::instance().subscribe<ActorTickEvent>([](ActorTickEvent& event) {
    LocalPlayer* player = event.localPlayer;
});
```

#### Publishing Events
```cpp
AttackEvent event(gameMode, localPlayer, target);
EventBus::instance().publish(event);
if (event.is_cancelled()) {
    // Attack was blocked by a module
}
```

#### Unsubscribing
```cpp
EventBus::instance().unsubscribe(id);
```

---

### 3. Accessing Minecraft EnTT Components

All actors wrap an `EntityContext` providing direct access to Bedrock's EnTT registry:

```cpp
// Check component existence
if (actor->hasComponent<StateVectorComponent>()) {
    auto* state = actor->getComponent<StateVectorComponent>();
    math::Vec3 position = state->mPos;
    math::Vec3 velocity = state->mPosDelta;
}

// Iterate over all active world actors safely on ActorTickEvent
for (Actor* actor : Actor::get_all(localPlayer)) {
    if (actor->isPlayer()) {
        // Process player entity
    }
}
```

---

### 4. Hooking Functions & Memory Signatures

#### Signature Scanning
Define patterns in `src/minecraft/bedrock_memory.h` and scan with `libhat`:

```cpp
const std::uintptr_t addr = hook_memory::resolve_signature("FunctionName", "48 89 5C 24 ? 57 48 83 EC");
```

---

### 5. Screen Projection (WorldToScreen)

Visual modules rendering in `on_imgui_render()` can project 3D coordinates to 2D screen positions via `ClientInstance`:

```cpp
#include "minecraft/sdk/client/client_instance.h"

math::Vec2 screen;
if (ClientInstance::get()->world_to_screen(worldPos, screen)) {
    // Render 2D overlays at (screen.x, screen.y)
    ImGui::GetBackgroundDrawList()->AddCircleFilled(
        ImVec2(screen.x, screen.y), 3.0f, IM_COL32(255, 255, 255, 255));
}
```

---

## Disclaimer

This project is provided strictly for educational and research purposes. Test and modify software only in authorized private development environments. You are solely responsible for adhering to applicable software licenses, game terms of service, and platform rules.

---

## License

This project is licensed under the [MIT License](LICENSE). You are permitted to use, modify, distribute, sublicense, and rebrand this software without restriction, provided that the original copyright and permission notices are retained.
