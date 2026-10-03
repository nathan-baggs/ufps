#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace ufps
{

struct MeshView
{
    std::uint32_t index_offset;
    std::uint32_t index_count;
    std::uint32_t vertex_offset;
    std::uint32_t vertex_count;

    auto operator==(const MeshView &) const -> bool = default;
};

}

template <>
struct std::hash<ufps::MeshView>
{
    auto operator()(const ufps::MeshView &obj) const noexcept -> std::size_t
    {
        auto seed = 0zu;
        auto combine = [&seed](std::uint32_t component)
        {
            const auto h = std::hash<std::uint32_t>{}(component);
            seed ^= h + std::size_t{0x9e3779b9} + (seed << 6) + (seed >> 2);
        };

        combine(obj.index_offset);
        combine(obj.index_count);
        combine(obj.vertex_offset);
        combine(obj.vertex_count);

        return seed;
    }
};
