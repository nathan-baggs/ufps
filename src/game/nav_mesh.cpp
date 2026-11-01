#include "game/nav_mesh.h"

#include <contracts>
#include <cstddef>
#include <numeric>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

#include "core/entity_manager.h"
#include "core/service_locator.h"
#include "maths/vector3.h"

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

auto NavMesh::route(std::size_t start_node_index, std::size_t end_node_index) -> std::vector<Node>
{
    struct AStar : Node
    {
        AStar(const Node &node, std::size_t index, float f = std::numeric_limits<float>::infinity())
            : Node{node}
            , f{f}
            , index{index}
        {
        }

        auto operator<(const AStar &other) const -> bool
        {
            return f < other.f;
        }

        float f;
        std::size_t index;
    };

    auto q = std::priority_queue<AStar>{};
    q.emplace(nodes_[start_node_index], start_node_index);

    auto seen = std::unordered_set<std::size_t>{start_node_index};

    auto g = std::unordered_map<std::size_t, float>{{start_node_index, 0.0f}};
    auto came_from = std::unordered_map<std::size_t, std::size_t>{};

    while (!std::ranges::empty(q))
    {
        const auto current = q.top();
        q.pop();

        if (current.index == end_node_index)
        {
            auto path = std::vector<Node>{nodes_[current.index]};

            auto cursor = current.index;
            while (cursor != start_node_index)
            {
                cursor = came_from[cursor];
                path.push_back(nodes_[cursor]);
            }

            std::ranges::reverse(path);

            return path;
        }

        for (const auto &neighbour : edges_[current.index])
        {
            const auto tentative_g = g[current.index] + Vector3::distance(current.position, nodes_[neighbour].position);

            const auto neighbour_g_find = g.find(neighbour);
            const auto neighbour_g = neighbour_g_find == std::ranges::cend(g) ? std::numeric_limits<float>::infinity()
                                                                              : neighbour_g_find->second;

            if (tentative_g < neighbour_g)
            {
                came_from[neighbour] = current.index;
                g[neighbour] = tentative_g;
                if (!seen.contains(neighbour))
                {
                    seen.insert(neighbour);
                    q.emplace(
                        nodes_[neighbour],
                        neighbour,
                        tentative_g + Vector3::distance(nodes_[neighbour].position, nodes_[end_node_index].position));
                }
            }
        }
    }

    return {};
}
}
