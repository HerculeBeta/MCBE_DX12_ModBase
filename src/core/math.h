#pragma once

#include <cmath>

namespace core {
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ViewAngles {
    float pitch = 0.0f;
    float yaw = 0.0f;
};

inline constexpr float kPi = 3.14159265358979323846f;

inline float degrees_to_radians(float degrees) {
    return degrees * (kPi / 180.0f);
}

inline Vec3 scale(const Vec3& value, float amount) {
    return {
        value.x * amount,
        value.y * amount,
        value.z * amount
    };
}

inline Vec3 direction_from_minecraft_view(const ViewAngles& angles) {
    const float yawRadians = degrees_to_radians(angles.yaw + 90.0f);
    const float pitchRadians = degrees_to_radians(-angles.pitch);

    return {
        std::cos(yawRadians) * std::cos(pitchRadians),
        std::sin(pitchRadians),
        std::sin(yawRadians) * std::cos(pitchRadians)
    };
}
}
