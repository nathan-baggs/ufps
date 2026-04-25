#pragma once

#include <algorithm>
#include <cstdint>

#include "graphics/mesh_manager.h"
#include "graphics/mesh_view.h"
#include "maths/aabb.h"

namespace ufps
{

namespace impl
{
constexpr auto calculate_aabb(ufps::MeshView mesh_view, const ufps::MeshManager &mesh_manager) -> ufps::AABB
{
    const auto vertices = mesh_manager.vertex_data(mesh_view);

    auto initial_aabb = ufps::AABB{
        .min = {std::numeric_limits<float>::max()},
        .max = {std::numeric_limits<float>::lowest()},
    };

    return std::ranges::fold_left(
        vertices,
        initial_aabb,
        [](const auto &a, const auto &b)
        {
            return ufps::AABB{
                .min =
                    {
                        std::min(a.min.x, b.position.x),
                        std::min(a.min.y, b.position.y),
                        std::min(a.min.z, b.position.z),
                    },
                .max =
                    {
                        std::max(a.max.x, b.position.x),
                        std::max(a.max.y, b.position.y),
                        std::max(a.max.z, b.position.z),
                    },
            };
        });
}
}

class RenderEntity
{
  public:
    constexpr RenderEntity(MeshView mesh_view, std::uint32_t material_index, const MeshManager &mesh_manager);

    constexpr auto mesh_view() const -> MeshView;
    constexpr auto material_index() const -> std::uint32_t;
    constexpr auto aabb() const -> const AABB &;

  private:
    MeshView mesh_view_;
    std::uint32_t material_index_;
    AABB aabb_;
};

constexpr RenderEntity::RenderEntity(MeshView mesh_view, std::uint32_t material_index, const MeshManager &mesh_manager)
    : mesh_view_{mesh_view}
    , material_index_{material_index}
    , aabb_{impl::calculate_aabb(mesh_view_, mesh_manager)}
{
}

constexpr auto RenderEntity::mesh_view() const -> MeshView
{
    return mesh_view_;
}

constexpr auto RenderEntity::material_index() const -> std::uint32_t
{
    return material_index_;
}

constexpr auto RenderEntity::aabb() const -> const AABB &
{
    return aabb_;
}

}
