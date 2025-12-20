#pragma once

#include <format>
#include <string>

#include "maths/vector3.h"

namespace ufps
{

class Vector4
{
  public:
    constexpr Vector4()
        : Vector4(0.0f)
    {
    }

    constexpr Vector4(float xyzw)
        : Vector4(xyzw, xyzw, xyzw, xyzw)
    {
    }

    constexpr Vector4(const Vector3 &v3, float w)
        : Vector4(v3.x, v3.y, v3.z, w)
    {
    }

    constexpr Vector4(float x, float y, float z, float w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
    }

    constexpr operator Vector3() const;

    constexpr auto operator==(const Vector4 &) const -> bool = default;

    constexpr auto to_string() const -> std::string;

    float x;
    float y;
    float z;
    float w;
};

constexpr Vector4::operator Vector3() const
{
    return Vector3{x, y, z};
}

constexpr auto Vector4::to_string() const -> std::string
{
    return std::format("x={} y={} z={} w={}", x, y, z, w);
}

}
