#include "game/nav_mesh.h"

#include <contracts>
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
             std::views::filter([](const auto &e) { return e.name().starts_with("floor") && e.name() != "floor"; }) |
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

    const auto width = static_cast<int>(std::sqrt(std::ranges::size(nodes_)));
    log::debug("width: {} count: {}", width, std::ranges::size(nodes_));

    static int index_range[] = {-1, 0, 1};

    for (auto x = 0; x < width; ++x)
    {
        for (auto y = 0; y < width; ++y)
        {
            const auto index = x + (y * width);

            for (const auto &pair : std::views::cartesian_product(index_range, index_range))
            {
                const auto &[dx, dy] = pair;

                if (dx == 0 && dy == 0)
                {
                    continue;
                }

                const auto next_x = x + dx;
                const auto next_y = y + dy;

                if (std::clamp(next_x, 0, width - 1) == next_x && std::clamp(next_y, 0, width - 1) == next_y)
                {
                    edges_[index].push_back(next_x + (next_y * width));
                }
            }
        }
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
