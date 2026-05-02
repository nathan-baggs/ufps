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
    constexpr RenderEntity(
        MeshView mesh_view,
        std::uint64_t albedo_texture_bindless_handle,
        std::uint64_t normal_texture_bindless_handle,
        std::uint64_t specular_texture_bindless_handle,
        const MeshManager &mesh_manager);

    constexpr auto mesh_view() const -> MeshView;
    constexpr auto albedo_texture_bindless_handle() const -> std::uint64_t;
    constexpr auto normal_texture_bindless_handle() const -> std::uint64_t;
    constexpr auto specular_texture_bindless_handle() const -> std::uint64_t;
    constexpr auto aabb() const -> const AABB &;

  private:
    MeshView mesh_view_;
    std::uint64_t albedo_texture_bindless_handle_;
    std::uint64_t normal_texture_bindless_handle_;
    std::uint64_t specular_texture_bindless_handle_;
    AABB aabb_;
};

constexpr RenderEntity::RenderEntity(
    MeshView mesh_view,
    std::uint64_t albedo_texture_bindless_handle,
    std::uint64_t normal_texture_bindless_handle,
    std::uint64_t specular_texture_bindless_handle,
    const MeshManager &mesh_manager)
    : mesh_view_{mesh_view}
    , albedo_texture_bindless_handle_{albedo_texture_bindless_handle}
    , normal_texture_bindless_handle_{normal_texture_bindless_handle}
    , specular_texture_bindless_handle_{specular_texture_bindless_handle}
    , aabb_{impl::calculate_aabb(mesh_view_, mesh_manager)}
{
}

constexpr auto RenderEntity::mesh_view() const -> MeshView
{
    return mesh_view_;
}

constexpr auto RenderEntity::albedo_texture_bindless_handle() const -> std::uint64_t
{
    return albedo_texture_bindless_handle_;
}

constexpr auto RenderEntity::normal_texture_bindless_handle() const -> std::uint64_t
{
    return normal_texture_bindless_handle_;
}

constexpr auto RenderEntity::specular_texture_bindless_handle() const -> std::uint64_t
{
    return specular_texture_bindless_handle_;
}

constexpr auto RenderEntity::aabb() const -> const AABB &
{
    return aabb_;
}

}
