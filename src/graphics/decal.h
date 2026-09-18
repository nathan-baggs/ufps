#pragma once

#include <cstdint>

#include "maths/matrix4.h"
#include "maths/transform.h"

namespace ufps
{

struct alignas(16) Decal
{
    constexpr Decal()
        : Decal({}, {})
    {
    }

    constexpr Decal(const Transform &transform, std::uint64_t decal_handle)
        : transform{transform}
        , inv_transform{Matrix4::invert(transform)}
        , decal_handle{decal_handle}
    {
    }

    Matrix4 transform;
    Matrix4 inv_transform;
    std::uint64_t decal_handle;
};
}
