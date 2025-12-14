#pragma once

#include "maths/vector3.h"

namespace ufps
{

struct VertexData
{
    Vector3 position;
};

static_assert(sizeof(VertexData) == sizeof(float) * 3);

}
