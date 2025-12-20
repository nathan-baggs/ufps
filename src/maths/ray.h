#pragma once

#include "maths/vector3.h"

namespace ufps
{

class Ray
{
  public:
    constexpr Ray(const Vector3 &origin, const Vector3 &direction)
        : origin{origin}
        , direction{Vector3::normalise(direction)}
    {
    }

    Vector3 origin;
    Vector3 direction;
};

}
