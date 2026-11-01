#include "game/nav_mesh.h"

#include <ranges>

#include "core/entity_manager.h"
#include "core/service_locator.h"

namespace ufps
{
NavMesh::NavMesh()
    : nodes_{}
    , edges_{}
{
    const auto &[em] = services<EntityManager>();

    nodes_ = em.data() | //
             std::views::filter([](const auto &e) { return e.name().starts_with("floor"); }) |
             std::views::transform(
                 [](const auto &e)
                 {
                     auto half_adj = (e.aabb().max + e.aabb().min) * 0.5f;
                     half_adj.y = 0.0f;

                     auto box = e.aabb();
                     box.min -= half_adj;
                     box.max -= half_adj;

                     return Node{
                         .box = box,
                         .position = e.transform().position + half_adj,
                     };
                 }) |
             std::ranges::to<std::vector>();

    for (auto i = 0zu; i < std::ranges::size(nodes_); ++i)
    {
        edges_[i] = {0zu};
    }
}

auto NavMesh::nodes() const -> std::span<const Node>
{
    return nodes_;
}

auto NavMesh::edges(std::size_t node_index) const -> std::span<const std::size_t>
{
    return edges_.find(node_index)->second;
}
}
