#pragma once

#include "minecraft/sdk/entity/i_entity_component.h"
#include <cstdint>
#include <type_traits>

struct RuntimeIDComponent : public IEntityComponent {
    std::uint64_t mRuntimeId = 0;
};

static_assert(sizeof(RuntimeIDComponent) == 0x8, "RuntimeIDComponent size mismatch!");
static_assert(std::is_standard_layout_v<RuntimeIDComponent>, "RuntimeIDComponent must be standard layout!");