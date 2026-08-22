#pragma once

#include "utils/math.h"
#include "minecraft/sdk/entity/i_entity_component.h"
#include <type_traits>

struct ActorHeadRotationComponent : public IEntityComponent {
    math::Vec2 mHeadYaw{ 0.0f, 0.0f };
};

static_assert(sizeof(ActorHeadRotationComponent) == 0x8, "ActorHeadRotationComponent size mismatch!");
static_assert(std::is_standard_layout_v<ActorHeadRotationComponent>, "ActorHeadRotationComponent must be standard layout!");