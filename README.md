# MCBE DX12 Internal Client Template

A clean C++ template for building an internal DirectX 12 client for Minecraft Bedrock Edition. This project is designed as an educational starting point for tutorial viewers who want to learn how a client can be structured, rendered, and extended.

The codebase focuses on a small, readable foundation: DX12 hooking, ImGui rendering, keybind handling, a module system, and tick event dispatch. It is intentionally general purpose so new systems and modules can be added without restructuring the project.

## Features

- DirectX 12 overlay rendering with ImGui
- Modular client architecture with categories, keybinds, settings, and lifecycle callbacks
- Win32 input routing for menu interaction and module keybinds
- Game-thread hook support
- CMake-based Visual Studio workflow

## Project Layout

```text
src/
  core/                    Shared math and small utility types
  events/                  Cross-system runtime event payloads
  input/                   Key names and module keybind handling
  minecraft/               Bedrock memory access helpers
    hooks/                 Minecraft hook installation and memory scanning helpers
  modules/                 Base module system and example module structure
    movement/              Movement module examples
  platform/windows/        DLL entry point and Windows-specific bootstrap
  render/dx12/             DX12 hook setup, ImGui lifecycle, and input forwarding
  runtime/                 Tick event dispatch code
  ui/                      Menu rendering and UI runtime settings

external/
  imgui/                   ImGui and platform/render backends
  kiero/                   Render hook bootstrap
  minhook/                 Function hook library
  d3d12/                   DX12 helper headers
```

## Requirements

- Windows 10 or newer
- Visual Studio 2022 with the Desktop development with C++ workload
- CMake 3.20 or newer
- A supported Minecraft Bedrock Edition build

## Getting Started

1. Open the folder in Visual Studio or another CMake-aware IDE.
2. Select an x64 configuration.
3. Build the shared library target.
4. Inject the produced DLL into the minecraft.windows.exe process using your preferred injector.

The default menu key is `INSERT`. The default unload key is `END`.

## Adding Modules

Create a new class that inherits from `Module`, place it under the matching `src/modules/<category>/` folder, register it in `ModuleManager`, and override only the callbacks your feature needs:

- `on_enable`
- `on_disable`
- `on_imgui_render`
- `on_settings_render`
- `on_normal_tick`

Project source files under `src/` are picked up by CMake automatically. Modules can expose settings in the menu, use hold or toggle keybind behavior, and live under any category you register.


## Disclaimer

This project is provided for educational and research purposes. Use it only in environments where you have permission to test and modify the running software. You are responsible for complying with applicable laws, game terms, and platform rules.
