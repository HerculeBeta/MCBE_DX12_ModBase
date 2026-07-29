#pragma once

#include "core/math.h"

#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace minecraft {
struct BedrockActorOffsets {
    static constexpr std::ptrdiff_t ActorMotionComponent = 0x218;
    static constexpr std::ptrdiff_t ActorRotationComponent = 0x228;

    static constexpr std::ptrdiff_t VelocityX = 0x18;
    static constexpr std::ptrdiff_t VelocityY = 0x1C;
    static constexpr std::ptrdiff_t VelocityZ = 0x20;

    static constexpr std::ptrdiff_t Pitch = 0x0;
    static constexpr std::ptrdiff_t Yaw = 0x4;
};

struct BedrockVtableSlots {
    static constexpr int ClientInstanceGetLocalPlayer = 31;
    static constexpr int ActorNormalTick = 24;
};

struct BedrockSignatures {
    static constexpr const char* ClientInstanceUpdate =
        "55 56 57 48 83 EC 50 48 8D 6C 24 50 48 C7 45 F8 FE FF FF FF 48 89 CE 48 8B 01 48 8B 80 68 03 00 00 48 8D 55 E0 FF 15 ?? ?? ?? ??";
};

namespace actor_motion {
inline std::uintptr_t resolve(void* actor) {
    if (!actor) {
        return 0;
    }

    __try {
        return *reinterpret_cast<std::uintptr_t*>(
            reinterpret_cast<std::uintptr_t>(actor) + BedrockActorOffsets::ActorMotionComponent);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

inline bool read_velocity(void* actor, core::Vec3& out) {
    const std::uintptr_t component = resolve(actor);
    if (!component) {
        return false;
    }

    __try {
        out.x = *reinterpret_cast<float*>(component + BedrockActorOffsets::VelocityX);
        out.y = *reinterpret_cast<float*>(component + BedrockActorOffsets::VelocityY);
        out.z = *reinterpret_cast<float*>(component + BedrockActorOffsets::VelocityZ);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

inline bool write_velocity(void* actor, const core::Vec3& value) {
    const std::uintptr_t component = resolve(actor);
    if (!component) {
        return false;
    }

    __try {
        *reinterpret_cast<float*>(component + BedrockActorOffsets::VelocityX) = value.x;
        *reinterpret_cast<float*>(component + BedrockActorOffsets::VelocityY) = value.y;
        *reinterpret_cast<float*>(component + BedrockActorOffsets::VelocityZ) = value.z;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
}

namespace actor_rotation {
inline std::uintptr_t resolve(void* actor) {
    if (!actor) {
        return 0;
    }

    __try {
        return *reinterpret_cast<std::uintptr_t*>(
            reinterpret_cast<std::uintptr_t>(actor) + BedrockActorOffsets::ActorRotationComponent);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

inline bool read_angles(void* actor, core::ViewAngles& out) {
    const std::uintptr_t component = resolve(actor);
    if (!component) {
        return false;
    }

    __try {
        out.pitch = *reinterpret_cast<float*>(component + BedrockActorOffsets::Pitch);
        out.yaw = *reinterpret_cast<float*>(component + BedrockActorOffsets::Yaw);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
}
}
