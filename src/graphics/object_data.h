#pragma once

#include <cstdint>

#include "maths/matrix4.h"

namespace ufps
{

struct ObjectData
{
    Matrix4 model;
    std::uint32_t material_id_index;
    std::uint32_t padding[3];
};

}
