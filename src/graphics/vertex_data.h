#pragma once

#include "graphics/colour.h"
#include "maths/vector3.h"

namespace ufps
{

struct VertexData
{
    Vector3 position;
    Colour colour;
};

static_assert(sizeof(VertexData) == sizeof(float) * 3 + sizeof(float) * 3);

}
