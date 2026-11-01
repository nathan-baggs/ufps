#pragma once

#include <contracts>
#include <cstddef>
#include <ranges>
#include <span>
#include <unordered_map>
#include <vector>

#include "maths/aabb.h"
#include "maths/vector3.h"

namespace ufps
{

struct Node
{
    AABB box;
    Vector3 position;
};

class NavMesh
{
  public:
    NavMesh();

    auto nodes() const -> std::span<const Node>;

    auto edges(std::size_t node_index) const -> std::span<const std::size_t> //
        pre(node_index < std::ranges::size(nodes_));

    auto route(std::size_t start_node_index, std::size_t end_node_index) -> std::vector<Node> //
        pre(start_node_index < std::ranges::size(nodes_))                                     //
        pre(end_node_index < std::ranges::size(nodes_))                                       //
        post(r : std::ranges::size(r) > 2zu);

  private:
    std::vector<Node> nodes_;
    std::unordered_map<std::size_t, std::vector<std::size_t>> edges_;
};

}
