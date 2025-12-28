#pragma once

#include "maths/vector3.h"

namespace ufps
{

struct UV
{
    float s;
    float t;
};

struct VertexData
{
    Vector3 position;
    UV uv;
};

static_assert(sizeof(VertexData) == sizeof(float) * 3 + sizeof(float) * 2);

}
