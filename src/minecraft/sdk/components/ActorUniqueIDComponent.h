#pragma once

#include "minecraft/sdk/entity/i_entity_component.h"
#include <cstdint>
#include <type_traits>

struct ActorUniqueIDComponent : public IEntityComponent {
    std::uint64_t mUniqueID = 0;
};

static_assert(sizeof(ActorUniqueIDComponent) == 0x8, "ActorUniqueIDComponent size mismatch!");
static_assert(std::is_standard_layout_v<ActorUniqueIDComponent>, "ActorUniqueIDComponent must be standard layout!");