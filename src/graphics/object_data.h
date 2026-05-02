#pragma once

#include <cstdint>

#include "maths/matrix4.h"

namespace ufps
{

struct alignas(16) ObjectData
{
    Matrix4 model;
    std::uint64_t albedo_texture_index;
    std::uint64_t normal_texture_index;
    std::uint64_t specular_texture_index;
};

}
