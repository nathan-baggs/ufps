#pragma once

#include <cstdint>

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
