#pragma once

#include <functional>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

#include "animation/keyframe.h"
#include "utils/string_map.h"

namespace ufps
{
namespace impl
{
auto all_keyframes_equal_length(const auto &key_frame_data) -> bool
{
    const auto vec_lengths =
        std::views::values(key_frame_data) | std::views::transform([](const auto &e) { return std::ranges::size(e); });

    return std::ranges::adjacent_find(vec_lengths, std::ranges::not_equal_to()) == std::ranges::cend(vec_lengths);
}
}
class Animation
{
  public:
    using KeyframeData = StringMap<std::vector<Keyframe>>;

    Animation(std::string name, KeyframeData keyframe_data) //
        pre(impl::all_keyframes_equal_length(keyframe_data));

    auto name() const -> std::string_view;

    auto keyframes(std::string_view name) const -> std::span<const Keyframe> //
        pre(keyframe_data_.contains(name));

    auto node_names() const -> std::span<const std::string_view>;

  private:
    std::string name_;
    StringMap<std::vector<Keyframe>> keyframe_data_;
    std::vector<std::string_view> node_names_;
};
}
