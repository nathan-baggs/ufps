#pragma once

#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "maths/matrix4.h"

namespace ufps
{

class Skeleton
{
  public:
    struct Node
    {
        std::string name;
        Matrix4 bind_local;
        Matrix4 inverse_bind;
        std::optional<std::size_t> parent;

        auto to_string() const -> std::string;
    };

    struct Description
    {
        std::string name;
        std::vector<Node> nodes;
    };

    Skeleton(std::string_view name, std::vector<Node> nodes) //
        post(validate());

    Skeleton(const Description &desc) //
        post(validate());

    auto name() const -> std::string_view;

    auto node(std::size_t index) const -> const Node & //
        pre(index < std::ranges::size(nodes_));

    auto validate() const -> bool;

    auto to_string() const -> std::string;

    auto description() const -> Description;

  private:
    std::string name_;
    std::vector<Node> nodes_;
};

}
