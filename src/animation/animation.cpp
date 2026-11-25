#include "animation/animation.h"

namespace ufps
{
Animation::Animation(std::string name, KeyframeData keyframe_data)
    : name_{std::move(name)}
    , keyframe_data_{std::move(keyframe_data)}
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

}
