#pragma once

#include <format>
#include <string>

namespace ufps
{

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

    auto to_string() const -> std::string;

    float x;
    float y;
    float z;
    float w;
};

inline auto Quaternion::to_string() const -> std::string
{
    return std::format("x={} y={} z={} w={}", x, y, z, w);
}

}
