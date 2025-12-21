#pragma once

#include <cstdint>
#include <span>

#include "graphics/vertex_data.h"

namespace ufps
{

struct MeshView
{
    std::uint32_t index_offset;
    std::uint32_t index_count;
    std::uint32_t vertex_offset;
    std::uint32_t vertex_count;
};

}
