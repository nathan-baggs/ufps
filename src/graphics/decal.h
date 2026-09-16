#pragma once

#include "maths/matrix4.h"
#include "maths/transform.h"

namespace ufps
{

struct Decal
{
    constexpr Decal(const Transform &transform)
        : transform{transform}
        , inv_transform{Matrix4::invert(transform)}
    {
    }

    Matrix4 transform;
    Matrix4 inv_transform;
};
}
