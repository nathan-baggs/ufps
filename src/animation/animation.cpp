#include "animation/animation.h"

#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace
{

auto extract_node_names(const auto &keyframe_data) -> std::vector<std::string_view>
{
    return std::views::keys(keyframe_data) | std::views::transform([](const auto &e) { return std::string_view{e}; }) |
           std::ranges::to<std::vector>();
}

}
namespace ufps
{
Animation::Animation(std::string name, KeyframeData keyframe_data)
    : name_{std::move(name)}
    , keyframe_data_{std::move(keyframe_data)}
    , node_names_{extract_node_names(keyframe_data_)}
{
}

auto Animation::name() const -> std::string_view
{
    return name_;
}

auto Animation::keyframes(std::string_view name) const -> std::span<const Keyframe>
{
    auto keyframes = keyframe_data_.find(name);
    return keyframes->second;
}

auto Animation::node_names() const -> std::span<const std::string_view>
{
    return node_names_;
}

}
