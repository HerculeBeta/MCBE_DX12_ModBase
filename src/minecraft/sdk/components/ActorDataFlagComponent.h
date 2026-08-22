#pragma once

#include "minecraft/sdk/entity/i_entity_component.h"
#include <cstddef>
#include <cstdint>

enum class ActorFlags : int {
    Onfire = 0,
    Sneaking = 1,
    Riding = 2,
    Sprinting = 3,
    Usingitem = 4,
    Invisible = 5,
    Tempted = 6,
    Inlove = 7,
    Saddled = 8,
    Powered = 9,
    Ignited = 10,
    Baby = 11,
    Converting = 12,
    Critical = 13,
    CanShowName = 14,
    AlwaysShowName = 15,
    Noai = 16,
    Silent = 17,
    Wallclimbing = 18,
    Canclimb = 19,
    Canswim = 20,
    Canfly = 21,
    Canwalk = 22,
    Resting = 23,
    Sitting = 24,
    Angry = 25,
    Interested = 26,
    Charged = 27,
    Tamed = 28,
    Orphaned = 29,
    Leashed = 30,
    Sheared = 31,
    Gliding = 32,
    Elder = 33,
    Moving = 34,
    Breathing = 35,
    Chested = 36,
    Stackable = 37,
    ShowBottom = 38,
    Standing = 39,
    Shaking = 40,
    Idling = 41,
    Casting = 42,
    Charging = 43,
    WasdControlled = 44,
    CanPowerJump = 45,
    CanDash = 46,
    Lingering = 47,
    HasCollision = 48,
    HasGravity = 49,
    FireImmune = 50,
    Dancing = 51,
    Enchanted = 52,
    Returntrident = 53,
    ContainerIsPrivate = 54,
    IsTransforming = 55,
    Damagenearbymobs = 56,
    Swimming = 57,
    Bribed = 58,
    IsPregnant = 59,
    LayingEgg = 60,
    PassengerCanPick = 61,
    TransitionSitting = 62,
    Eating = 63,
    LayingDown = 64,
    Sneezing = 65,
    Trusting = 66,
    Rolling = 67,
    Scared = 68,
    InScaffolding = 69,
    OverScaffolding = 70,
    DescendThroughBlock = 71,
    Blocking = 72,
    TransitionBlocking = 73,
    BlockedUsingShield = 74,
    BlockedUsingDamagedShield = 75,
    Sleeping = 76,
    WantsToWake = 77,
    TradeInterest = 78,
    DoorBreaker = 79,
    BreakingObstruction = 80,
    DoorOpener = 81,
    IsIllagerCaptain = 82,
    Stunned = 83,
    Roaring = 84,
    DelayedAttack = 85,
    IsAvoidingMobs = 86,
    IsAvoidingBlock = 87,
    FacingTargetToRangeAttack = 88,
    HiddenWhenInvisible = 89,
    IsInUi = 90,
    Stalking = 91,
    Emoting = 92,
    Celebrating = 93,
    Admiring = 94,
    CelebratingSpecial = 95,
    OutOfControl = 96,
    RamAttack = 97,
    PlayingDead = 98,
    InAscendableBlock = 99,
    OverDescendableBlock = 100,
    Croaking = 101,
    EatMob = 102,
    JumpGoalJump = 103,
    Emerging = 104,
    Sniffing = 105,
    Digging = 106,
    SonicBoom = 107,
    HasDashCooldown = 108,
    PushTowardsClosestSpace = 109,
    Deprecated1 = 110,
    Deprecated2 = 111,
    Deprecated3 = 112,
    Searching = 113,
    Crawling = 114,
    TimerFlag1 = 115,
    TimerFlag2 = 116,
    TimerFlag3 = 117,
    BodyRotationBlocked = 118,
    RendersWhenInvisible = 119,
    RotationAxisAligned = 120,
    Collidable = 121,
    WasdFreeCameraControlled = 122,
    DoesServerAuthOnlyDismount = 123,
    BodyRotationAlwaysFollowsHead = 124,
    CanUseVerticalMovementAction = 125,
    RotationLockedToVehicle = 126,
    UsesLegacyFriction = 127,
    UsesUniformAirDrag = 128,
    NameplateDepthTested = 129,
    Count = 130
};

namespace brstd {
    template <std::size_t N, typename Word = std::uint64_t>
    struct bitset {
        static constexpr std::size_t kBitsPerWord = sizeof(Word) * 8;
        static constexpr std::size_t kWords = (N + kBitsPerWord - 1) / kBitsPerWord; 

        Word mStorage[kWords]{};

        [[nodiscard]] bool test(std::size_t index) const {
            if (index >= N) return false;
            return (mStorage[index / kBitsPerWord] & (Word(1) << (index % kBitsPerWord))) != 0;
        }

        void set(std::size_t index, bool value = true) {
            if (index >= N) return;
            if (value) {
                mStorage[index / kBitsPerWord] |= (Word(1) << (index % kBitsPerWord));
            }
            else {
                mStorage[index / kBitsPerWord] &= ~(Word(1) << (index % kBitsPerWord));
            }
        }
    };
}

template <typename T>
struct ActorDataComponentBase {
    T mValue{};
};

struct ActorDataFlagComponent : public IEntityComponent, public ActorDataComponentBase<brstd::bitset<130, std::uint64_t>> {
    [[nodiscard]] bool getFlag(ActorFlags flag) const {
        return mValue.test(static_cast<std::size_t>(flag));
    }

    void setFlag(ActorFlags flag, bool value) {
        mValue.set(static_cast<std::size_t>(flag), value);
    }
};

static_assert(sizeof(brstd::bitset<130, std::uint64_t>) == 0x18, "brstd::bitset size mismatch!");
static_assert(sizeof(ActorDataFlagComponent) == 0x18, "ActorDataFlagComponent size mismatch!");