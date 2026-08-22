#pragma once

#include "utils/math.h"
#include "minecraft/sdk/entity/i_entity_component.h"
#include <type_traits>

struct RenderPositionComponent : public IEntityComponent {
    math::Vec3 mPosition{ 0.0f, 0.0f, 0.0f };
};

static_assert(sizeof(RenderPositionComponent) == 0x0C, "RenderPositionComponent size mismatch!");
static_assert(std::is_standard_layout_v<RenderPositionComponent>, "RenderPositionComponent must be standard layout!");