#pragma once

#include <format>

namespace ufps
{

struct Colour
{
    float r;
    float g;
    float b;
};

namespace colours
{
inline constexpr Colour white{.r = 1.0f, .g = 1.0f, .b = 1.0f};
inline constexpr Colour azure{.r = 0.0f, .g = 0.5f, .b = 1.0f};
}

inline auto to_string(const Colour &obj) -> std::string
{
    return std::format("r={} g={} b={}", obj.r, obj.g, obj.b);
}

}
