#pragma once

#include <optional>
#include <string>

#include "graphics/opengl.h"
#include "utils/auto_release.h"

namespace ufps
{

enum class FilterType
{
    LINEAR_MIPMAP,
    LINEAR,
    NEAREST
};

enum class WrapMode
{
    REPEAT,
    CLAMP_TO_EDGE,
    MIRRORED_REPEAT
};

class Sampler
{
  public:
    Sampler(
        FilterType min_filter,
        FilterType mag_filter,
        WrapMode wrap_s,
        WrapMode wrap_t,
        const std::string &name,
        std::optional<float> anisotropy_samples = std::nullopt);

    auto native_handle() const -> ::GLuint;

    auto name() const -> std::string;

  private:
    AutoRelease<::GLuint> handle_;
    std::string name_;
};

}
