#pragma once

#include <string>

#include "maths/vector3.h"
#include "utils/formatter.h"

namespace ufps
{
struct AABB
{
    Vector3 min;
    Vector3 max;
};

inline auto to_string(const AABB &obj) -> std::string
{
    return std::format("min: {} max: {}", obj.min, obj.max);
}

}
