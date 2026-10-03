#pragma once

#include <format>

#include "maths/vector3.h"
#include "utils/formatter.h"

namespace ufps
{

struct Cone
{
    Vector3 origin;
    Vector3 extent;
    float theta;
};

inline auto to_string(const Cone &cone)
{
    return std::format("origin: {} extent: {} theta: {}", cone.origin, cone.extent, cone.theta);
}

}
