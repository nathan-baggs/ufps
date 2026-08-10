#pragma once

#include <format>
#include <string>

#include "maths/vector3.h"

namespace ufps
{

namespace impl
{

}

class Quaternion
{
  public:
    constexpr Quaternion()
        : Quaternion(0.0f, 0.0f, 0.0f, 1.0f)
    {
    }

    constexpr Quaternion(float x, float y, float z, float w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
    }

    constexpr Quaternion(const Vector3 &axis, float angle)
    {
        const auto sin_half = std::sin(angle / 2.0f);
        const auto cos_half = std::cos(angle / 2.0f);

        x = axis.x * sin_half;
        y = axis.y * sin_half;
        z = axis.z * sin_half;
        w = cos_half;

        *this = normalise(*this);
    }

    constexpr Quaternion(float yaw, float pitch, float roll);

    static constexpr auto normalise(const Quaternion &q) -> Quaternion
    {
        const auto det = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        contract_assert(det != 0.0f);

        return {q.x / det, q.y / det, q.z / det, q.w / det};
    }

    auto to_string() const -> std::string;

    float x;
    float y;
    float z;
    float w;
};

constexpr auto operator*(const Quaternion &a, const Quaternion &b) -> Quaternion
{
    // thanks Harrand!
    return {
        (b.w * a.x) + (b.z * a.y) + (b.y * -a.z) + (b.x * a.w),
        (b.z * -a.x) + (b.w * a.y) + (b.x * a.z) + (b.y * a.w),
        (b.y * a.x) + (b.x * -a.y) + (b.w * a.z) + (b.z * a.w),
        (b.x * -a.x) + (b.y * -a.y) + (b.z * -a.z) + (b.w * a.w)};
}

constexpr Quaternion::Quaternion(float yaw, float pitch, float roll)
{
    const auto qx = Quaternion{{1.0f, 0.0f, 0.0f}, yaw};
    const auto qy = Quaternion{{0.0f, 1.0f, 0.0f}, pitch};
    const auto qz = Quaternion{{0.0f, 0.0f, 1.0f}, roll};

    *this = normalise(qz * qy * qx);
}

inline auto Quaternion::to_string() const -> std::string
{
    return std::format("x={} y={} z={} w={}", x, y, z, w);
}

}
