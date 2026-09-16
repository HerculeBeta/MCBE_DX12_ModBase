#pragma once

#include <cmath>

namespace math {
    inline constexpr float kPi = 3.14159265358979323846f;

    struct Vec2 {
        float x = 0.0f, y = 0.0f;

        Vec2 operator*(float s) const { return { x * s, y * s }; }
        Vec2 operator/(float s) const { return s != 0.0f ? Vec2{ x / s, y / s } : Vec2{}; }
        Vec2 operator+(const Vec2& o) const { return { x + o.x, y + o.y }; }
        Vec2 operator-(const Vec2& o) const { return { x - o.x, y - o.y }; }
        Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
        Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
        Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

        float length() const { return std::sqrt(x * x + y * y); }
        float length_sq() const { return x * x + y * y; }
        float magnitude() const { return length(); }

        Vec2 normalized() const {
            const float len = length();
            return len > 0.0001f ? *this / len : Vec2{};
        }

        float distance(const Vec2& o) const { return (*this - o).length(); }
        float distance_sq(const Vec2& o) const { return (*this - o).length_sq(); }
    };

    struct Vec3 {
        float x = 0.0f, y = 0.0f, z = 0.0f;

        Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
        Vec3 operator/(float s) const { return s != 0.0f ? Vec3{ x / s, y / s, z / s } : Vec3{}; }
        Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
        Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
        Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
        Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
        Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

        float length() const { return std::sqrt(x * x + y * y + z * z); }
        float length_sq() const { return x * x + y * y + z * z; }
        float magnitude() const { return length(); }

        Vec3 normalized() const {
            const float len = length();
            return len > 0.0001f ? *this / len : Vec3{};
        }

        float distance(const Vec3& o) const { return (*this - o).length(); }
        float distance_sq(const Vec3& o) const { return (*this - o).length_sq(); }
    };

    struct ViewAngles {
        float pitch = 0.0f;
        float yaw = 0.0f;
    };

    inline Vec3 get_direction(ViewAngles angles) {
        const float pitchRad = angles.pitch * (kPi / 180.0f);
        const float yawRad = angles.yaw * (kPi / 180.0f);
        const float cosPitch = std::cos(pitchRad);

        return {
            -std::sin(yawRad) * cosPitch,
            -std::sin(pitchRad),
             std::cos(yawRad) * cosPitch
        };
    }

    inline void get_horizontal_vectors(float yaw, Vec3& forward, Vec3& right) {
        const float yawRad = yaw * (kPi / 180.0f);
        forward = { -std::sin(yawRad), 0.0f, std::cos(yawRad) };
        right = { -forward.z, 0.0f, forward.x };
    }
}

using Vec2 = math::Vec2;
using Vec3 = math::Vec3;
using ViewAngles = math::ViewAngles;