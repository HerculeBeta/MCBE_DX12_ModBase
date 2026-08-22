#pragma once

#include <cstdint>

namespace minecraft::hooks {
    bool install_client_instance_hook(std::uintptr_t updateAddr);
    void remove_client_instance_hook();
}