#include "minecraft/hooks/actor_tick_hook.h"
#include "minecraft/sdk/client/client_instance.h"
#include "events/event_bus.h"
#include "events/actor_tick_event.h"
#include "minecraft/sdk/actor/local_player.h"
#include "utils/logger.h"

#include <MinHook.h>
#include <atomic>
#include <mutex>

namespace {
    using ActorTickFn = void(__fastcall*)(void* self);
    ActorTickFn g_originalActorTick = nullptr;
    void* g_actorTickTarget = nullptr;
    std::uintptr_t g_normalTickAddr = 0;

    std::atomic<void*> g_currentLocalPlayer{ nullptr };
    std::atomic<void*> g_currentClientInstance{ nullptr };
    std::mutex g_hookMutex;

    void __fastcall hook_actor_normal_tick(void* self) {
        if (g_originalActorTick) {
            g_originalActorTick(self);
        }

        void* expectedPlayer = g_currentLocalPlayer.load(std::memory_order_acquire);
        void* clientInst = g_currentClientInstance.load(std::memory_order_acquire);

        if (self != nullptr && expectedPlayer != nullptr && self == expectedPlayer && clientInst != nullptr) {
            ClientInstance::setLocalPlayer(static_cast<LocalPlayer*>(self));
            ActorTickEvent event(static_cast<ClientInstance*>(clientInst), static_cast<LocalPlayer*>(self));
            EventBus::instance().publish(event);
        }
    }
}

namespace minecraft::hooks {
    void set_actor_tick_address(std::uintptr_t normalTickAddr) {
        g_normalTickAddr = normalTickAddr;
    }

    void install_actor_tick_hook(void* localPlayer, void* clientInstance) {
        g_currentLocalPlayer.store(localPlayer, std::memory_order_release);
        g_currentClientInstance.store(clientInstance, std::memory_order_release);

        if (!localPlayer) return;

        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (g_actorTickTarget != nullptr || !g_normalTickAddr) {
            return;
        }

        g_actorTickTarget = reinterpret_cast<void*>(g_normalTickAddr);
        if (MH_CreateHook(g_actorTickTarget, reinterpret_cast<LPVOID>(&hook_actor_normal_tick), reinterpret_cast<void**>(&g_originalActorTick)) == MH_OK) {
            MH_EnableHook(g_actorTickTarget);
            LOG_INFO("Actor::normalTick hook attached successfully.");
        }
        else {
            LOG_ERROR("Failed to create Actor::normalTick hook!");
            g_actorTickTarget = nullptr;
        }
    }

    void remove_actor_tick_hook() {
        std::lock_guard<std::mutex> lock(g_hookMutex);

        ClientInstance::setLocalPlayer(nullptr);
        g_currentLocalPlayer.store(nullptr, std::memory_order_release);
        g_currentClientInstance.store(nullptr, std::memory_order_release);

        if (g_actorTickTarget) {
            MH_DisableHook(g_actorTickTarget);
            MH_RemoveHook(g_actorTickTarget);
            g_actorTickTarget = nullptr;
            g_originalActorTick = nullptr;
        }
    }
}