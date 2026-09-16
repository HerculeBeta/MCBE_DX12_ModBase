#include "minecraft/hooks/client_instance_hook.h"
#include "minecraft/sdk/client/client_instance.h"
#include "events/event_bus.h"
#include "events/client_instance_update_event.h"
#include "utils/logger.h"

#include <MinHook.h>

namespace {
    using ClientUpdateFn = bool(__fastcall*)(void* self, bool isInitFinished);
    ClientUpdateFn g_originalClientUpdate = nullptr;
    void* g_clientUpdateTarget = nullptr;

    bool __fastcall hook_client_instance_update(void* self, bool isInitFinished) {
        auto* instance = static_cast<ClientInstance*>(self);
        if (instance) {
            ClientInstance::set(instance);
        }

        ClientInstanceUpdateEvent event(instance, isInitFinished);
        EventBus::instance().publish(event);

        return g_originalClientUpdate ? g_originalClientUpdate(self, isInitFinished) : false;
    }
}

namespace minecraft::hooks {
    bool install_client_instance_hook(std::uintptr_t updateAddr) {
        if (!updateAddr) {
            LOG_ERROR("Invalid ClientInstance::update address provided!");
            return false;
        }

        LOG_INFO("Hooking ClientInstance::update at address: {:#x}", updateAddr);

        g_clientUpdateTarget = reinterpret_cast<void*>(updateAddr);
        if (MH_CreateHook(g_clientUpdateTarget, &hook_client_instance_update, reinterpret_cast<void**>(&g_originalClientUpdate)) != MH_OK) {
            LOG_ERROR("Failed to create ClientInstance::update hook!");
            g_clientUpdateTarget = nullptr;
            return false;
        }

        if (MH_EnableHook(g_clientUpdateTarget) != MH_OK) {
            LOG_ERROR("Failed to enable ClientInstance::update hook!");
            return false;
        }

        LOG_INFO("ClientInstance update hook enabled.");
        return true;
    }

    void remove_client_instance_hook() {
        ClientInstance::set(nullptr);
        if (g_clientUpdateTarget) {
            MH_DisableHook(g_clientUpdateTarget);
            MH_RemoveHook(g_clientUpdateTarget);
            g_clientUpdateTarget = nullptr;
            g_originalClientUpdate = nullptr;
            LOG_INFO("ClientInstance update hook removed.");
        }
    }
}