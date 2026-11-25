#include "animation/skeleton.h"

#include <algorithm>
#include <format>
#include <ranges>
#include <sstream>
#include <string_view>
#include <vector>

#include "animation/animation.h"
#include "maths/matrix4.h"
#include "utils/formatter.h"
#include "utils/string_map.h"

namespace
{

auto extract_animation_names(const auto &animations) -> std::vector<std::string_view>
{
    return std::views::keys(animations) | std::views::transform([](const auto &e) { return std::string_view{e}; }) |
           std::ranges::to<std::vector>();
}

}

namespace ufps
{

auto Skeleton::Node::to_string() const -> std::string
{
    return std::format("{} {}", name, parent.transform([](auto e) { return std::to_string(e); }).value_or("<no name>"));
}

Skeleton::Skeleton(std::string_view name, std::vector<Node> nodes, StringMap<Animation> animations)
    : name_{name}
    , nodes_{std::move(nodes)}
    , animations_{std::move(animations)}
    , animation_names_{extract_animation_names(animations_)}
{
}

Skeleton::Skeleton(std::string_view name, std::span<Node> nodes, StringMap<Animation> animations)
    : name_{name}
    , nodes_{std::ranges::cbegin(nodes), std::ranges::cend(nodes)}
    , animations_{std::move(animations)}
    , animation_names_{extract_animation_names(animations_)}
{
}

auto Skeleton::name() const -> std::string_view
{
    return name_;
}

auto Skeleton::node(std::size_t index) const -> const Node &
{
    return nodes_[index];
}

auto Skeleton::nodes() const -> std::span<const Node>
{
    return nodes_;
}

auto Skeleton::validate() const -> bool
{
    if (std::ranges::empty(nodes_))
    {
        return false;
    }

    if (nodes_.front().parent)
    {
        return false;
    }

    return std::ranges::all_of(
        std::views::enumerate(nodes_) | std::views::drop(1zu),
        [](const auto &e)
        {
            const auto &[index, node] = e;
            return node.parent && *(node.parent) < static_cast<std::size_t>(index);
        });
}

auto Skeleton::to_string() const -> std::string
{
    auto strm = std::stringstream{};

    strm << std::format("skeleton: {}\n", name_);

    for (const auto &node : nodes_)
    {
        strm << std::format("{}\n", node);
    }

    return strm.str();
}

auto Skeleton::animation(std::string_view name) const -> const Animation &
{
    auto animation = animations_.find(name);
    return animation->second;
}

auto Skeleton::animation_names() const -> std::span<const std::string_view>
{
    return animation_names_;
}

}
