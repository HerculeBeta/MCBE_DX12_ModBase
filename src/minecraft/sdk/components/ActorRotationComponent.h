#pragma once

#include "utils/math.h"
#include "minecraft/sdk/entity/i_entity_component.h"
#include <type_traits>

struct ActorRotationComponent : public IEntityComponent {
    math::ViewAngles mRotation{ 0.0f, 0.0f };
    math::ViewAngles mOldRotation{ 0.0f, 0.0f };
};

static_assert(sizeof(ActorRotationComponent) == 0x10, "ActorRotationComponent size mismatch!");
static_assert(std::is_standard_layout_v<ActorRotationComponent>, "ActorRotationComponent must be standard layout!");