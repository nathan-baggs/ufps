#include "animation/skeleton.h"

#include <algorithm>
#include <format>
#include <ranges>
#include <sstream>
#include <string_view>
#include <vector>

#include "maths/matrix4.h"
#include "utils/formatter.h"

namespace ufps
{

auto Skeleton::Node::to_string() const -> std::string
{
    return std::format("{} {}", name, parent.transform([](auto e) { return std::to_string(e); }).value_or("<no name>"));
}

Skeleton::Skeleton(std::string_view name, std::vector<Node> nodes)
    : name_{name}
    , nodes_{std::move(nodes)}
{
}

Skeleton::Skeleton(const Description &desc)
    : Skeleton{desc.name, desc.nodes}
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

auto Skeleton::description() const -> Description
{
    return {
        .name = name_,
        .nodes = nodes_,
    };
}
}
