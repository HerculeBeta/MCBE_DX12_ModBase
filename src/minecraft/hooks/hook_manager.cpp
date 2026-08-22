#include "minecraft/hooks/hook_manager.h"
#include "minecraft/hooks/client_instance_hook.h"
#include "minecraft/hooks/actor_tick_hook.h"
#include "minecraft/hooks/attack_hook.h"
#include "minecraft/hooks/hook_memory.h"
#include "minecraft/bedrock_memory.h"
#include "events/event_bus.h"
#include "events/client_instance_update_event.h"
#include "utils/logger.h"

#include <MinHook.h>

namespace minecraft::hooks {

    namespace {
        std::uintptr_t s_getLocalPlayerAddr = 0;

        void on_client_update(ClientInstanceUpdateEvent& event) {
            if (!s_getLocalPlayerAddr) return;

            void* localPlayer = hook_memory::call_function<void*>(
                s_getLocalPlayerAddr,
                event.clientInstance);

            if (localPlayer) {
                install_actor_tick_hook(localPlayer, event.clientInstance);
                install_attack_hook(static_cast<LocalPlayer*>(localPlayer));
            }
        }
    }

    bool initialize() {
        LOG_INFO("Initializing MinHook framework...");
        if (MH_Initialize() != MH_OK) {
            LOG_ERROR("Failed to initialize MinHook!");
            return false;
        }

        LOG_INFO("Resolving game signatures...");

        const std::uintptr_t updateAddr = hook_memory::resolve_signature(
            "ClientInstance::update",
            BedrockSignatures::clientInstance_Update);

        s_getLocalPlayerAddr = hook_memory::resolve_signature(
            "ClientInstance::getLocalPlayer",
            BedrockSignatures::clientInstance_getLocalPlayer);

        const std::uintptr_t actorTickAddr = hook_memory::resolve_signature(
            "Actor::normalTick",
            BedrockSignatures::actor_normalTick);

        if (!updateAddr || !s_getLocalPlayerAddr || !actorTickAddr) {
            LOG_FATAL("One or more required signatures failed to resolve!");
            return false;
        }

        // Pass the scanned signature address once
        set_actor_tick_address(actorTickAddr);

        EventBus::instance().subscribe<ClientInstanceUpdateEvent>(on_client_update);

        if (!install_client_instance_hook(updateAddr)) {
            LOG_ERROR("Failed to install ClientInstance::update hook!");
            return false;
        }

        LOG_INFO("All game hooks initialized successfully.");
        return true;
    }

    void shutdown() {
        LOG_INFO("Shutting down game hooks...");
        remove_attack_hook();
        remove_actor_tick_hook();
        remove_client_instance_hook();
        MH_Uninitialize();
    }

}