#pragma once

#include "graphics/colour.h"
#include "maths/vector3.h"

namespace ufps
{

struct PointLight
{
    Vector3 position;
    Colour colour;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};

static_assert(sizeof(PointLight) == sizeof(float) * 9);

}
