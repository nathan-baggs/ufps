#include "core/sub_mesh.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <ranges>

#include "graphics/mesh_manager.h"
#include "graphics/mesh_view.h"
#include "maths/aabb.h"

namespace
{
auto calculate_aabb(ufps::MeshView mesh_view, const ufps::MeshManager &mesh_manager) -> ufps::AABB
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

namespace ufps
{
SubMesh::SubMesh(MeshView mesh_view, std::uint32_t material_index, const MeshManager &mesh_manager)
    : mesh_view_{mesh_view}
    , material_index_{material_index}
    , aabb_{calculate_aabb(mesh_view_, mesh_manager)}
{
}

auto SubMesh::mesh_view() const -> MeshView
{
    return mesh_view_;
}

auto SubMesh::material_index() const -> std::uint32_t
{
    return material_index_;
}

auto SubMesh::aabb() const -> const AABB &
{
    return aabb_;
}

}
