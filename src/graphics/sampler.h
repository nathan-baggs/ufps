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

class Sampler
{
  public:
    Sampler(
        FilterType min_filter,
        FilterType mag_filter,
        const std::string &name,
        std::optional<float> anisotropy_samples = std::nullopt);

    auto native_handle() const -> ::GLuint;

    auto name() const -> std::string;

  private:
    AutoRelease<::GLuint> handle_;
    std::string name_;
};

inline auto to_string(FilterType filter_type) -> std::string
{
    switch (filter_type)
    {
        using enum ufps::FilterType;

        case LINEAR_MIPMAP: return "LINEAR_MIPMAP";
        case LINEAR: return "LINEAR";
        case NEAREST: return "NEAREST";
        default: return "<unknown>";
    }
}

}
