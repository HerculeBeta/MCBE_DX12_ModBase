#pragma once

#include "utils/math.h"
#include "minecraft/sdk/entity/i_entity_component.h"

#include <array>
#include <cstddef>
#include <cstdint>

#pragma pack(push, 1)

struct MoveInputState {
    std::uint64_t mBits = 0;
    std::uint64_t mReserved = 0;
};
static_assert(sizeof(MoveInputState) == 0x10, "MoveInputState size mismatch!");

struct MoveInputComponent : public IEntityComponent {
    enum class Flag : int {
        Sneaking = 0,
        Sprinting = 1,
        WantUp = 2,
        WantDown = 3,
        Jumping = 4,
        AutoJumpingInWater = 5,
        MoveInputStateLocked = 6,
        PersistSneak = 7,
        AutoJumpEnabled = 8,
        IsCameraRelativeMovementEnabled = 9,
        IsRotControlledByMoveDirection = 10,
        Count = 11
    };

    MoveInputState mInputState{};
    MoveInputState mRawInputState{};
    std::uint8_t mHoldAutoJumpInWaterTicks = 0;
    std::uint8_t pad_0x21[0x03]{};
    Vec2 mMove{};
    Vec2 mLookDelta{};
    Vec2 mInteractDir{};
    Vec3 mDisplacement{};
    Vec3 mDisplacementDelta{};
    Vec3 mCameraOrientation{};
    std::uint16_t mFlagValues = 0;
    std::array<bool, 2> mIsPaddling{};

    [[nodiscard]] bool getFlag(Flag flag) const {
        const int index = static_cast<int>(flag);
        return index >= 0 && index < static_cast<int>(Flag::Count) && ((mFlagValues >> index) & 1u) != 0;
    }

    void setFlag(Flag flag, bool value) {
        const int index = static_cast<int>(flag);
        if (index < 0 || index >= static_cast<int>(Flag::Count)) return;
        const std::uint16_t mask = static_cast<std::uint16_t>(1u << index);
        if (value) mFlagValues = static_cast<std::uint16_t>(mFlagValues | mask);
        else mFlagValues = static_cast<std::uint16_t>(mFlagValues & ~mask);
    }

    void reset(bool lockMove = false) {
        mInputState = {};
        mRawInputState = {};
        mMove = {};
        mLookDelta = {};
        mInteractDir = {};
        mDisplacement = {};
        mDisplacementDelta = {};
        mCameraOrientation = {};
        mFlagValues = 0;
        mIsPaddling = {};
        setFlag(Flag::MoveInputStateLocked, lockMove);
    }
};
static_assert(sizeof(MoveInputComponent) == 0x64, "MoveInputComponent size mismatch!");

struct RawMoveInputComponent : public IEntityComponent {
    MoveInputState mRawInput{};
    Vec2 mRawMove{};
};
static_assert(sizeof(RawMoveInputComponent) == 0x18, "RawMoveInputComponent size mismatch!");

#pragma pack(pop)