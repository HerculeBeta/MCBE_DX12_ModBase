#include "minecraft/hooks/client_instance_hook.h"

#include "minecraft/bedrock_memory.h"
#include "minecraft/hooks/hook_memory.h"
#include "runtime/tick_dispatcher.h"

#include <Windows.h>
#include <MinHook.h>

#include <cstdint>

namespace {
using ClientUpdateFn = bool(__fastcall*)(void* self, bool isInitFinished);
using ActorTickFn = void(__fastcall*)(void* self);

ClientUpdateFn g_originalClientUpdate = nullptr;
ActorTickFn g_originalActorTick = nullptr;

void* g_clientInstance = nullptr;
void* g_localPlayer = nullptr;
void* g_clientUpdateTarget = nullptr;
void* g_actorTickTarget = nullptr;

void* get_local_player(void* clientInstance) {
    return minecraft::hook_memory::call_vtable<void*>(
        clientInstance,
        minecraft::BedrockVtableSlots::ClientInstanceGetLocalPlayer);
}

void __fastcall hook_actor_normal_tick(void* self) {
    if (g_originalActorTick) {
        g_originalActorTick(self);
    }

    if (self == g_localPlayer && g_clientInstance) {
        tick_dispatcher::dispatch_normal_tick(g_clientInstance, g_localPlayer);
    }
}

void install_actor_normal_tick_hook() {
    if (!g_localPlayer || g_actorTickTarget) {
        return;
    }

    const std::uintptr_t normalTick = minecraft::hook_memory::vtable_entry(
        g_localPlayer,
        minecraft::BedrockVtableSlots::ActorNormalTick);
    if (!normalTick) {
        return;
    }

    g_actorTickTarget = reinterpret_cast<void*>(normalTick);
    if (MH_CreateHook(g_actorTickTarget, reinterpret_cast<LPVOID>(&hook_actor_normal_tick), reinterpret_cast<void**>(&g_originalActorTick)) == MH_OK) {
        MH_EnableHook(g_actorTickTarget);
    } else {
        g_actorTickTarget = nullptr;
    }
}

bool __fastcall hook_client_instance_update(void* self, bool isInitFinished) {
    g_clientInstance = self;
    g_localPlayer = get_local_player(self);
    install_actor_normal_tick_hook();

    const bool result = g_originalClientUpdate
        ? g_originalClientUpdate(self, isInitFinished)
        : false;

    g_localPlayer = get_local_player(self);
    install_actor_normal_tick_hook();

    return result;
}
}

namespace minecraft {
bool start_client_instance_hook() {
    const MH_STATUS initStatus = MH_Initialize();
    if (initStatus != MH_OK && initStatus != MH_ERROR_ALREADY_INITIALIZED) {
        return false;
    }

    const std::uintptr_t update = hook_memory::find_signature(BedrockSignatures::ClientInstanceUpdate);
    if (!update) {
        return false;
    }

    g_clientUpdateTarget = reinterpret_cast<void*>(update);
    if (MH_CreateHook(
            g_clientUpdateTarget,
            &hook_client_instance_update,
            reinterpret_cast<void**>(&g_originalClientUpdate)) != MH_OK) {
        g_clientUpdateTarget = nullptr;
        return false;
    }

    if (MH_EnableHook(g_clientUpdateTarget) != MH_OK) {
        MH_RemoveHook(g_clientUpdateTarget);
        g_clientUpdateTarget = nullptr;
        return false;
    }

    return true;
}

void stop_client_instance_hook() {
    if (g_actorTickTarget) {
        MH_DisableHook(g_actorTickTarget);
        MH_RemoveHook(g_actorTickTarget);
    }

    if (g_clientUpdateTarget) {
        MH_DisableHook(g_clientUpdateTarget);
        MH_RemoveHook(g_clientUpdateTarget);
    }

    g_actorTickTarget = nullptr;
    g_clientUpdateTarget = nullptr;
    g_originalActorTick = nullptr;
    g_originalClientUpdate = nullptr;
    g_clientInstance = nullptr;
    g_localPlayer = nullptr;
}

void* client_instance() {
    return g_clientInstance;
}

void* local_player() {
    return g_localPlayer;
}
}
