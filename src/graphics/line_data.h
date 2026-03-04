#pragma once

#include <string>

#include "graphics/colour.h"
#include "maths/vector3.h"

#include "utils/formatter.h"

namespace ufps
{
struct LineData
{
    Vector3 position;
    Colour colour;
};

inline auto to_string(const LineData &obj) -> std::string
{
    return std::format("{} {}", obj.position, obj.colour);
}

}
