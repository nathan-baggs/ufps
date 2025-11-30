#pragma once

#include <cmath>
#include <format>

namespace ufps
{

struct Vector3
{
    constexpr Vector3()
        : Vector3(0.0f)
    {
    }

    constexpr Vector3(float xyz)
        : Vector3(xyz, xyz, xyz)
    {
    }

    constexpr Vector3(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
    {
    }

    constexpr auto length() const -> float
    {
        return std::hypot(x, y, z);
    }

    static constexpr auto normalise(const Vector3 &v) -> Vector3
    {
        const auto l = v.length();
        if (l == 0.0f)
        {
            return {};
        }
        return {v.x / l, v.y / l, v.z / l};
    }

    static constexpr auto cross(const Vector3 &v1, const Vector3 &v2) -> Vector3
    {
        const auto i = (v1.y * v2.z) - (v1.z * v2.y);
        const auto j = (v1.x * v2.z) - (v1.z * v2.x);
        const auto k = (v1.x * v2.y) - (v1.y * v2.x);

        return {i, -j, k};
    }

    static constexpr auto dot(const Vector3 &v1, const Vector3 &v2) -> float
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    static constexpr auto distance(const Vector3 &v1, const Vector3 &v2) -> float;

    constexpr auto operator==(const Vector3 &) const -> bool = default;

    constexpr auto to_string() const -> std::string;

    float x;
    float y;
    float z;
};

constexpr auto operator-=(Vector3 &v1, const Vector3 &v2) -> Vector3 &
{
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;

    return v1;
}

constexpr auto operator-(const Vector3 &v1, const Vector3 &v2) -> Vector3
{
    auto tmp = v1;
    return tmp -= v2;
}

constexpr auto operator+=(Vector3 &v1, const Vector3 &v2) -> Vector3 &
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;

    return v1;
}

constexpr auto operator+(const Vector3 &v1, const Vector3 &v2) -> Vector3
{
    auto tmp = v1;
    return tmp += v2;
}

constexpr auto operator*=(Vector3 &v1, const Vector3 &v2) -> Vector3 &
{
    v1.x *= v2.x;
    v1.y *= v2.y;
    v1.z *= v2.z;

    return v1;
}

constexpr auto operator*(const Vector3 &v1, const Vector3 &v2) -> Vector3
{
    auto tmp = v1;
    return tmp *= v2;
}

constexpr auto operator/=(Vector3 &v1, const Vector3 &v2) -> Vector3 &
{
    v1.x /= v2.x;
    v1.y /= v2.y;
    v1.z /= v2.z;

    return v1;
}

constexpr auto operator/(const Vector3 &v1, const Vector3 &v2) -> Vector3
{
    auto tmp = v1;
    return tmp /= v2;
}

constexpr auto operator-(const Vector3 &v) -> Vector3
{
    return {-v.x, -v.y, -v.z};
}

constexpr auto Vector3::distance(const Vector3 &v1, const Vector3 &v2) -> float
{
    return (v2 - v1).length();
}

constexpr auto Vector3::to_string() const -> std::string
{
    return std::format("x={} y={} z={}", x, y, z);
}

}
