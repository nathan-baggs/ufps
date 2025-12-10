#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include "graphics/vertex_data.h"

namespace ufps
{

struct MeshData
{
    std::vector<VertexData> vertices;
    std::vector<std::uint32_t> indices;
};

inline auto to_string(const MeshData &data) -> std::string
{
    return std::format("mesh data: v: {} i: {}", data.vertices.size(), data.indices.size());
}

}
