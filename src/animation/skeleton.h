#pragma once

#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "animation/animation.h"
#include "maths/matrix4.h"
#include "utils/string_map.h"

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

    Skeleton(std::string_view name, std::vector<Node> nodes, StringMap<Animation> animations) //
        post(validate());

    Skeleton(std::string_view name, std::span<Node> nodes, StringMap<Animation> animations) //
        post(validate());

    auto name() const -> std::string_view;

    auto node(std::size_t index) const -> const Node & //
        pre(index < std::ranges::size(nodes_));

    auto nodes() const -> std::span<const Node>;

    auto validate() const -> bool;

    auto to_string() const -> std::string;

    auto animation(std::string_view name) const -> const Animation & //
        pre(animations_.contains(name));

    auto animation_names() const -> std::span<const std::string_view>;

  private:
    std::string name_;
    std::vector<Node> nodes_;
    StringMap<Animation> animations_;
    std::vector<std::string_view> animation_names_;
};

}
