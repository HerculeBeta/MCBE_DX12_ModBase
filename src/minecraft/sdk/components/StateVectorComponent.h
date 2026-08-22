#pragma once

#include "utils/math.h"
#include "minecraft/sdk/entity/i_entity_component.h"

struct StateVectorComponent : public IEntityComponent {
    math::Vec3 mPos;
    math::Vec3 mPosPrev;
    math::Vec3 mPosDelta;
};

static_assert(sizeof(StateVectorComponent) == 0x24, "StateVectorComponent size mismatch!");