#pragma once

#include <cstdint>

namespace minecraft {

    struct BedrockVtableSlots {
        static constexpr int gameMode_Attack = 15; //26.40
    };

    struct BedrockOffsets {
        static constexpr std::uintptr_t localPlayer_gameMode = 0xAA0; //26.40
        static constexpr std::uintptr_t clientInstance_levelRenderer = 0x1B8; //26.40
        static constexpr std::uintptr_t clientInstance_glMatrix = 0x418; //26.40
        static constexpr std::uintptr_t clientInstance_guiData = 0x648; //26.40
        static constexpr std::uintptr_t levelRenderer_renderPlayer = 0x468; //26.40
        static constexpr std::uintptr_t renderPlayer_cameraPos = 0x660; //26.40
        static constexpr std::uintptr_t renderPlayer_fovX = 0xF58; //26.40
        static constexpr std::uintptr_t renderPlayer_fovY = 0xF6C; //26.40
        static constexpr std::uintptr_t guiData_windowSize = 0x40; //26.40
    };

    struct BedrockSignatures {
        static constexpr const char* clientInstance_Update = //26.40
            "55 41 57 41 56 41 55 41 54 56 57 53 48 81 EC ? ? ? ? 48 8D AC 24 ? ? ? ? 48 C7 85 ? ? ? ? FE FF FF FF 89 D3 48 89 CE 48 8B 01 48 8B 80 ? ? ? ? 48 8D 95 ? ? ? ? FF 15 ? ? ? ?";
        static constexpr const char* clientInstance_getLocalPlayer = //26.40
            "48 83 EC 48 48 8B 05 ? ? ? ? 48 31 E0 48 89 44 24 40 48 8D 91 ? ? ? ? 48 8D 4C 24 20 E8 ? ? ? ? 80 7C 24 38 01";
        static constexpr const char* actor_normalTick = //26.40
            "55 41 57 41 56 41 55 41 54 56 57 53 48 81 EC ? ? ? ? 48 8D AC 24 ? ? ? ? 0F 29 BD ? ? ? ? 0F 29 B5 ? ? ? ? 48 C7 85 ? ? ? ? FE FF FF FF 48 89 CE 80 B9 ? ? ? ? 01 75 ? 83 BE ? ? ? ? 00";
    };

}