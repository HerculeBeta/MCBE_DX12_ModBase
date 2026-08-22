#pragma once

#include <cstdint>

namespace minecraft::hooks {
    void set_actor_tick_address(std::uintptr_t normalTickAddr);
    void install_actor_tick_hook(void* localPlayer, void* clientInstance);
    void remove_actor_tick_hook();
}