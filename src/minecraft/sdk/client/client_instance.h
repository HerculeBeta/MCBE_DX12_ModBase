#pragma once

#include "minecraft/bedrock_memory.h"
#include "minecraft/sdk/client/gl_matrix.h"
#include "utils/math.h"

#include <atomic>
#include <cstdint>

class LocalPlayer;

class GuiData {
public:
    [[nodiscard]] math::Vec2 getWindowSize() const {
        if (!this) return { 0.0f, 0.0f };
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::guiData_windowSize;
            if (addr < 0x10000) return { 0.0f, 0.0f };
            return *reinterpret_cast<const math::Vec2*>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return { 0.0f, 0.0f };
        }
    }
};

class LevelRendererPlayer {
public:
    [[nodiscard]] math::Vec3 getOrigin() const {
        if (!this) return { 0.0f, 0.0f, 0.0f };
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::renderPlayer_cameraPos;
            if (addr < 0x10000) return { 0.0f, 0.0f, 0.0f };
            return *reinterpret_cast<const math::Vec3*>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return { 0.0f, 0.0f, 0.0f };
        }
    }

    [[nodiscard]] float getFovX() const {
        if (!this) return 1.0f;
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::renderPlayer_fovX;
            if (addr < 0x10000) return 1.0f;
            return *reinterpret_cast<const float*>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return 1.0f;
        }
    }

    [[nodiscard]] float getFovY() const {
        if (!this) return 1.0f;
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::renderPlayer_fovY;
            if (addr < 0x10000) return 1.0f;
            return *reinterpret_cast<const float*>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return 1.0f;
        }
    }
};

class LevelRenderer {
public:
    [[nodiscard]] LevelRendererPlayer* getLevelRendererPlayer() const {
        if (!this) return nullptr;
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::levelRenderer_renderPlayer;
            if (addr < 0x10000) return nullptr;
            return *reinterpret_cast<LevelRendererPlayer**>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }
};

class ClientInstance {
public:
    static ClientInstance* get() {
        return s_instance.load(std::memory_order_acquire);
    }

    static void set(ClientInstance* instance) {
        s_instance.store(instance, std::memory_order_release);
    }

    [[nodiscard]] LevelRenderer* getLevelRenderer() const {
        if (!this) return nullptr;
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::clientInstance_levelRenderer;
            if (addr < 0x10000) return nullptr;
            return *reinterpret_cast<LevelRenderer**>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

    [[nodiscard]] GuiData* getGuiData() const {
        if (!this) return nullptr;
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::clientInstance_guiData;
            if (addr < 0x10000) return nullptr;
            return *reinterpret_cast<GuiData**>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

    [[nodiscard]] GLMatrix getGLMatrix() const {
        if (!this) return {};
        __try {
            const auto addr = reinterpret_cast<std::uintptr_t>(this) + minecraft::BedrockOffsets::clientInstance_glMatrix;
            if (addr < 0x10000) return {};
            return *reinterpret_cast<const GLMatrix*>(addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return {};
        }
    }

    [[nodiscard]] bool world_to_screen(const math::Vec3& worldPos, math::Vec2& screenPos) const;

private:
    static inline std::atomic<ClientInstance*> s_instance{ nullptr };
};
