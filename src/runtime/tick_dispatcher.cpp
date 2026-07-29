#include "runtime/tick_dispatcher.h"

#include "modules/module_manager.h"

namespace tick_dispatcher {
void dispatch_normal_tick(void* clientInstance, void* localPlayer) {
    NormalTickContext context{};
    context.clientInstance = clientInstance;
    context.localPlayer = localPlayer;

    ModuleManager::instance().on_normal_tick(context);
}
}
