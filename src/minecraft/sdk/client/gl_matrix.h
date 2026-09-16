#pragma once

#include "utils/math.h"

struct GLMatrix {
    float matrix[16]{};

    [[nodiscard]] math::Vec3 transform(const math::Vec3& p) const {
        return {
            matrix[0] * p.x + matrix[1] * p.y + matrix[2] * p.z + matrix[3],
            matrix[4] * p.x + matrix[5] * p.y + matrix[6] * p.z + matrix[7],
            matrix[8] * p.x + matrix[9] * p.y + matrix[10] * p.z + matrix[11]
        };
    }
};
