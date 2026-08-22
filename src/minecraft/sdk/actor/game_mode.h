#pragma once

#include "utils/math.h"
#include <cstdint>

class Actor;

class GameMode {
public:
    virtual ~GameMode() = default;                                                                                   // 0  (0x00)
    virtual bool startDestroyBlock(const void* pos, std::uint8_t face, bool& isDestroyedOut) = 0;                    // 1  (0x08)
    virtual bool destroyBlock(const void* pos, std::uint8_t face) = 0;                                               // 2  (0x10)
    virtual void continueDestroyBlock(const void* pos, std::uint8_t face, const Vec3& playerPos, bool& out) = 0;    // 3  (0x18)
    virtual void stopDestroyBlock(const void* pos) = 0;                                                              // 4  (0x20)
    virtual void startBuildBlock(const void* pos, std::uint8_t face) = 0;                                           // 5  (0x28)
    virtual bool buildBlock(const void* pos, std::uint8_t face, bool isAuthed) = 0;                                 // 6  (0x30)
    virtual void continueBuildBlock(const void* pos, std::uint8_t face) = 0;                                        // 7  (0x38)
    virtual void stopBuildBlock() = 0;                                                                               // 8  (0x40)
    virtual void tick() = 0;                                                                                         // 9  (0x48)
    virtual float getPickRange(const void* inputMode, bool isVR) = 0;                                                // 10 (0x50)
    virtual bool useItem(void* itemStack) = 0;                                                                       // 11 (0x58)
    virtual bool useItemAsAttack(void* itemStack) = 0;                                                               // 12 (0x60)
    virtual bool useItemOn(void* itemStack, const void* pos, std::uint8_t face, const Vec3& clickPos, void* block) = 0; // 13 (0x68)
    virtual bool interact(Actor* target, const Vec3& location) = 0;                                                  // 14 (0x70)
    virtual void attack(Actor* target) = 0;                                                                          // 15 (0x78)
    virtual void releaseUsingItem() = 0;                                                                             // 16 (0x80)
};