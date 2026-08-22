#include "minecraft/hooks/attack_hook.h"
#include "minecraft/hooks/hook_memory.h"
#include "minecraft/bedrock_memory.h"
#include "minecraft/sdk/actor/game_mode.h"
#include "minecraft/sdk/actor/local_player.h"
#include "events/event_bus.h"
#include "events/attack_event.h"
#include "utils/memory_utils.h"
#include "utils/logger.h"

#include <MinHook.h>
#include <atomic>
#include <mutex>
#include <chrono>

namespace {
    using GameModeAttackFn = void(__fastcall*)(void* self, void* target);
    GameModeAttackFn g_originalAttack = nullptr;
    std::uintptr_t g_currentHookedAddress = 0;

    std::atomic<LocalPlayer*> g_currentLocalPlayer{ nullptr };
    std::mutex g_attackMutex;
    std::chrono::steady_clock::time_point g_lastHookCheck;

    // Passive observer hook
    void __fastcall hook_gamemode_attack(void* self, void* target) {
        if (g_originalAttack) {
            g_originalAttack(self, target);
        }

        if (self && target) {
            LOG_INFO("[AttackHook] Attack triggered on target: {:p}", target);

            LocalPlayer* player = g_currentLocalPlayer.load(std::memory_order_relaxed);
            AttackEvent event(reinterpret_cast<GameMode*>(self), player, reinterpret_cast<Actor*>(target));
            EventBus::instance().publish(event);
        }
    }
}

namespace minecraft::hooks {
    void install_attack_hook(LocalPlayer* localPlayer) {
        g_currentLocalPlayer.store(localPlayer, std::memory_order_relaxed);

        if (!utils::memory::is_valid_ptr(localPlayer)) {
            return;
        }

        GameMode* gameMode = localPlayer->getGameMode();
        if (!utils::memory::is_valid_ptr(gameMode)) {
            return;
        }

        auto now = std::chrono::steady_clock::now();
        if (g_currentHookedAddress != 0 && std::chrono::duration_cast<std::chrono::seconds>(now - g_lastHookCheck).count() < 2) {
            return;
        }
        g_lastHookCheck = now;

        const std::uintptr_t funcAddr = hook_memory::vtable_entry(gameMode, BedrockVtableSlots::gameMode_Attack);
        if (!utils::memory::is_valid_ptr(reinterpret_cast<void*>(funcAddr))) {
            return;
        }

        std::lock_guard<std::mutex> lock(g_attackMutex);
        if (g_currentHookedAddress == funcAddr) {
            return;
        }

        if (g_currentHookedAddress != 0) {
            MH_DisableHook(reinterpret_cast<LPVOID>(g_currentHookedAddress));
            MH_RemoveHook(reinterpret_cast<LPVOID>(g_currentHookedAddress));
            g_currentHookedAddress = 0;
            g_originalAttack = nullptr;
        }

        if (MH_CreateHook(reinterpret_cast<LPVOID>(funcAddr), reinterpret_cast<LPVOID>(&hook_gamemode_attack), reinterpret_cast<void**>(&g_originalAttack)) == MH_OK) {
            if (MH_EnableHook(reinterpret_cast<LPVOID>(funcAddr)) == MH_OK) {
                g_currentHookedAddress = funcAddr;
                LOG_INFO("[AttackHook] GameMode::attack hooked successfully at {:#x}", funcAddr);
            }
        }
    }

    void remove_attack_hook() {
        std::lock_guard<std::mutex> lock(g_attackMutex);
        g_currentLocalPlayer.store(nullptr, std::memory_order_relaxed);

        if (g_currentHookedAddress != 0) {
            MH_DisableHook(reinterpret_cast<LPVOID>(g_currentHookedAddress));
            MH_RemoveHook(reinterpret_cast<LPVOID>(g_currentHookedAddress));
            g_currentHookedAddress = 0;
            g_originalAttack = nullptr;
            LOG_INFO("[AttackHook] GameMode::attack unhooked.");
        }
    }
}