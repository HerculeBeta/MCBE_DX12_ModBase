#pragma once

#include "utils/math.h"
#include "minecraft/sdk/entity/i_entity_component.h"
#include <type_traits>

struct AABBShapeComponent : public IEntityComponent {
    math::Vec3 mMin{ 0.0f, 0.0f, 0.0f };
    math::Vec3 mMax{ 0.0f, 0.0f, 0.0f };
    float mWidth = 0.0f;
    float mHeight = 0.0f;

    [[nodiscard]] math::Vec3 center() const {
        return {
            (mMin.x + mMax.x) * 0.5f,
            (mMin.y + mMax.y) * 0.5f,
            (mMin.z + mMax.z) * 0.5f
        };
    }

    [[nodiscard]] math::Vec3 extents() const {
        return {
            (mMax.x - mMin.x) * 0.5f,
            (mMax.y - mMin.y) * 0.5f,
            (mMax.z - mMin.z) * 0.5f
        };
    }
};

static_assert(sizeof(AABBShapeComponent) == 0x20, "AABBShapeComponent size mismatch!");
static_assert(std::is_standard_layout_v<AABBShapeComponent>, "AABBShapeComponent must be standard layout!");