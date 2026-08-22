#pragma once

#include "actor.h"
#include "minecraft/bedrock_memory.h"
#include "minecraft/sdk/actor/game_mode.h"

class LocalPlayer : public Actor {
public:
    [[nodiscard]] GameMode* getGameMode() const {
        if (!this) return nullptr;
        __try {
            const auto ptr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::localPlayer_gameMode;
            if (ptr < 0x10000) return nullptr;
            return *reinterpret_cast<GameMode**>(ptr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }
};