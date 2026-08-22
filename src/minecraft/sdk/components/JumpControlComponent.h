#pragma once

#include "minecraft/sdk/entity/i_entity_component.h"
#include <cstddef>
#include <cstdint>

enum class ActorJumpType : std::int32_t {
    Normal = 0
};

struct JumpData {
    std::uint8_t data[0x11]{};
};
static_assert(sizeof(JumpData) == 0x11, "JumpData size mismatch!");

struct JumpControlComponent : public IEntityComponent {
    bool mJumping = false;                             // 0x00
    bool mSwimming = false;                            // 0x01
    std::uint8_t pad_0x02[0x02]{};                    // 0x02
    float mJumpPower = 0.0f;                          // 0x04
    ActorJumpType mJumpType = ActorJumpType::Normal;   // 0x08
    JumpData mJumpData[4]{};                          // 0x0C
    void* mJumpControl = nullptr;                      // 0x50

    void setNoJumpDelay(bool value) {
        mJumping = value;
    }
};

static_assert(offsetof(JumpControlComponent, mJumping) == 0x0, "mJumping offset mismatch!");
static_assert(offsetof(JumpControlComponent, mSwimming) == 0x1, "mSwimming offset mismatch!");
static_assert(offsetof(JumpControlComponent, mJumpPower) == 0x4, "mJumpPower offset mismatch!");
static_assert(offsetof(JumpControlComponent, mJumpType) == 0x8, "mJumpType offset mismatch!");
static_assert(offsetof(JumpControlComponent, mJumpData) == 0xC, "mJumpData offset mismatch!");
static_assert(offsetof(JumpControlComponent, mJumpControl) == 0x50, "mJumpControl offset mismatch!");
static_assert(sizeof(JumpControlComponent) == 0x58, "JumpControlComponent size mismatch!");